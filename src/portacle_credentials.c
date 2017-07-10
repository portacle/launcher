#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL2_IMPLEMENTATION
#include "toolkit.c"
#include <GLFW/glfw3.h>
#include <gcrypt.h>
#include "nuklear.h"
#include "nuklear_glfw_gl2.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 150
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

static void error_callback(int e, const char *d){
  printf("Error %d: %s\n", e, d);
}

struct credentials{
  char protocol[64];
  char username[64];
  char password[64];
  char host[256];
  char path[256];
};

struct credentials *make_credentials(){
  struct credentials *credentials = calloc(1, sizeof(struct credentials));
  return credentials;
}

void free_credentials(struct credentials *credentials){
  free(credentials);
}

int credentials_show_dialog(struct credentials *creds){
  static GLFWwindow *win;
  struct nk_context *ctx;
  int width = 0, height = 0;
  int exit = 0;
  char root[PATHLEN] = {0};
  char info[1024] = {0};
  
  get_env("ROOT", root);
  strcat(info, "Connection info for ");
  strcat(info, creds->protocol);
  strcat(info, "://");
  strcat(info, creds->host);
  strcat(info, creds->path);

  glfwSetErrorCallback(error_callback);
  if(!glfwInit()) {
    fprintf(stdout, "[GFLW] failed to init!\n");
    return 0;
  }
  
  win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Portacle Credentials Storage", NULL, NULL);
  glfwMakeContextCurrent(win);
  glfwGetWindowSize(win, &width, &height);

  ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
  {
    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    if(root[0]){
      char path[PATHLEN];
      pathcat(path, root, 3, "all", "fonts", "NotoSansUI-Regular.ttf");
      struct nk_font *font = nk_font_atlas_add_from_file(atlas, path, 14, 0);
      nk_glfw3_font_stash_end();
      nk_style_set_font(ctx, &font->handle);
    }else{
      nk_glfw3_font_stash_end();
    }
  }
  
  while(!glfwWindowShouldClose(win)){
    int status = -1;
    glfwPollEvents();
    glfwGetWindowSize(win, &width, &height);
    nk_glfw3_new_frame();

    if(nk_begin(ctx, info, nk_rect(0, 0, width, height), 0)){
      nk_layout_row_dynamic(ctx, 30, 1);
      nk_label(ctx, info, NK_TEXT_LEFT);

      if(nk_input_is_key_pressed(&ctx->input, NK_KEY_TAB)){
        ctx->current->edit.name = (ctx->current->edit.name+1) % ctx->current->edit.old;
        ctx->current->edit.active = nk_true;
      }
      
      nk_layout_row_dynamic(ctx, 30, 2);
      nk_label(ctx, "Username:", NK_TEXT_LEFT);
      if(nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, creds->username, 64, nk_filter_default) & NK_EDIT_COMMITED){
        status = 1;
      }

      nk_layout_row_dynamic(ctx, 30, 2);
      nk_label(ctx, "Password:", NK_TEXT_LEFT);
      if(nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, creds->password, 64, nk_filter_default) & NK_EDIT_COMMITED){
        status = 1;
      }

      nk_layout_row_static(ctx, 30, 80, 2);
      if(nk_button_label(ctx, "Ok")){
        status = 1;
      }

      if(nk_button_label(ctx, "Cancel")){
        status = 0;
      }

      switch(status){
      case 0:
        glfwSetWindowShouldClose(win, GL_TRUE);
        break;
      case 1:
        if(creds->username[0] && creds->password[0]){
          glfwSetWindowShouldClose(win, GL_TRUE);
          exit = 1;
        }
        break;
      }
    }
    nk_end(ctx);

    glfwGetWindowSize(win, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    nk_glfw3_render(NK_ANTI_ALIASING_ON);
    glfwSwapBuffers(win);
  }
  
  nk_glfw3_shutdown();
  glfwTerminate();
  return exit;
}

#define CRYPT_KEY "D2io3j13(_aDn%+é9Ddl30'9ä$acEDöYdlkm-_.wa2124A/zlkm°21'@|4lk+!}a"
#define CRYPT_CIPHER GCRY_CIPHER_AES128
#define CRYPT_MODE GCRY_CIPHER_MODE_CBC
int credentials_encrypt(char *in_buffer, char *out_buffer){
  size_t key_length = gcry_cipher_get_algo_keylen(CRYPT_CIPHER);
  size_t blk_length = gcry_cipher_get_algo_blklen(CRYPT_CIPHER);
  size_t length = strlen(in_buffer)+1;
  gcry_error_t error = {0};
  gcry_cipher_hd_t handle = {0};
  char key[512] = {0};
  
  if(length % blk_length != 0) length += blk_length - length % blk_length;
  if(!get_env("PORTACLE_CREDENTIALS_KEY", key)) strcpy(key, CRYPT_KEY);

  error = gcry_cipher_open(&handle, CRYPT_CIPHER, CRYPT_MODE, 0);
  if(error) goto gcrypt_failure;

  error = gcry_cipher_setkey(handle, CRYPT_KEY, key_length);
  if(error) goto gcrypt_failure;

  error = gcry_cipher_encrypt(handle, out_buffer, length, in_buffer, length);
  if(error) goto gcrypt_failure;

  gcry_cipher_close(handle);
  return 1;
  
 gcrypt_failure:
  gcry_cipher_close(handle);
  fprintf(stderr, "GCRYPT Failure: %s\n", gcry_strerror(error));
  return 0;
}

int credentials_decrypt(char *in_buffer, char *out_buffer){
  size_t key_length = gcry_cipher_get_algo_keylen(CRYPT_CIPHER);
  size_t length = strlen(in_buffer);
  gcry_error_t error;
  gcry_cipher_hd_t handle;
  char key[512] = {0};
  
  if(!get_env("PORTACLE_CREDENTIALS_KEY", key)) strcpy(key, CRYPT_KEY);

  error = gcry_cipher_open(&handle, CRYPT_CIPHER, CRYPT_MODE, 0);
  if(error) goto gcrypt_failure;

  error = gcry_cipher_setkey(handle, CRYPT_KEY, key_length);
  if(error) goto gcrypt_failure;

  error = gcry_cipher_decrypt(handle, out_buffer, length, in_buffer, length);
  if(error) goto gcrypt_failure;

  gcry_cipher_close(handle);
  return 1;
  
 gcrypt_failure:
  gcry_cipher_close(handle);
  fprintf(stderr, "GCRYPT Failure: %s\n", gcry_strerror(error));
  return 0;
}

int credentials_read(FILE *stream, struct credentials *out, int decrypt){
  char *line = 0;
  size_t size = 0;

  out->protocol[0] = 0;
  out->host[0] = 0;
  out->path[0] = 0;
  out->username[0] = 0;
  out->password[0] = 0;
  
  // According to https://www.kernel.org/pub/software/scm/git/docs/git-credential.html
  // both EOF and empty line designate an end.
  while(1 < getline(&line, &size, stream)){
    line[strlen(line)-1] = 0;
    char *key = strtok(line, "=");
    char *val = strtok(0, "=");
    if(!key || !val){
      free(line);
      return 0;
    }
    
    /* */ if(streq(key, "protocol")){
      strncpy(out->protocol, val, 64);
    }else if(streq(key, "host")){
      strncpy(out->host, val, 256);
    }else if(streq(key, "path")){
      strncpy(out->path, val, 256);
    }else if(streq(key, "username")){
      strncpy(out->username, val, 64);
    }else if(streq(key, "password")){
      if(decrypt){
        char password[64] = {0};
        if(!credentials_decrypt(val, password)){
          free(line);
          return 0;
        }
        strncpy(out->password, password, 64);
      }else{
        strncpy(out->password, val, 64);
      }
    }else{
      fprintf(stderr, "Warning: Unknown credentials key %s, ignoring.\n", key);
    }
  }

  if(line) free(line);
  return 1;
}

int credentials_write(FILE *stream, struct credentials *out, int encrypt){
  if(out->protocol[0] != 0) fprintf(stream, "protocol=%s\n", out->protocol);
  if(out->host[0] != 0) fprintf(stream, "host=%s\n", out->host);
  if(out->path[0] != 0) fprintf(stream, "path=%s\n", out->path);
  if(out->username[0] != 0) fprintf(stream, "username=%s\n", out->username);
  if(out->password[0] != 0){
    if(encrypt){
      char password[64] = {0};
      if(!credentials_encrypt(out->password, password)){
        return 1;
      }
      fprintf(stream, "password=%s\n", password);
    }else{
      fprintf(stream, "password=%s\n", out->password);
    }
  }
  fprintf(stream, "\n");
  return 1;
}

int credentials_path(char *path){
  char root[PATHLEN] = {0};
  if(!get_env("ROOT", root) && !find_root(root)){
    return 0;
  }
  pathcat(path, root, 2, "config", ".credentials");
  return 1;
}

int credentials_load(struct credentials ***_creds){
  struct credentials **creds = 0;
  FILE *file = 0;
  char path[PATHLEN] = {0};
  size_t count = 0;
  size_t size = 64;

  *_creds = 0;
  creds = calloc(size, sizeof(struct credentials*));
  if(!creds)
    goto cleanup;

  if(!credentials_path(path))
    goto cleanup;
  
  if(!(file = fopen(path, "r")))
    goto cleanup;

  do{
    creds[count] = calloc(1, sizeof(struct credentials));
    if(!creds[count])
      goto cleanup;
    
    if(!credentials_read(file, creds[count], 1))
      goto cleanup;
    
    // Maybe resize vector
    ++count;
    if(size <= count+1){
      size *= 2;
      creds = realloc(creds, size*sizeof(struct credentials*));
      if(!creds) goto cleanup;
    }
  }while(!feof(file));
  creds[count] = 0;
  
  fclose(file);
  *_creds = creds;
  return 1;

 cleanup:
  if(file) fclose(file);
  if(creds){
    for(; 0 < count; --count){
      if(creds[count])free(creds[count]);
    }
    free(creds);
  }
  *_creds = 0;
  return 0;
}

int credentials_save(struct credentials **creds){
  FILE *file = 0;
  char path[PATHLEN] = {0};

  if(!credentials_path(path))
    goto cleanup;

  if(!(file = fopen(path, "w")))
    goto cleanup;

  for(size_t i=0; creds[i]; ++i){
    if(!credentials_write(file, creds[i], 1))
      goto cleanup;
  }

  fclose(file);
  return 1;

 cleanup:
  if(file) fclose(file);
  return 0;
}

#define FIELDEQ(a,b,f) (!a->f[0] || (b->f[0] && streq(a->f, b->f)))
int credentials_get(struct credentials *match, struct credentials **out){
  struct credentials **creds = 0;
  *out = 0;
  
  if(!credentials_load(&creds)){
    fprintf(stderr, "Failed to load credentials storage.\n");
    return 1;
  }

  for(size_t i=0; creds[i]; ++i){
    struct credentials *cred = creds[i];
    if(FIELDEQ(match, cred, protocol) &&
       FIELDEQ(match, cred, username) &&
       FIELDEQ(match, cred, password) &&
       FIELDEQ(match, cred, host) &&
       FIELDEQ(match, cred, path)){
      *out = cred;
      break;
    }
  }
  
  return 1;
}

int credentials_set(struct credentials *out){
  struct credentials **creds = 0;
  
  if(!credentials_load(&creds)){
    // Empty storage; create anew.
    creds = calloc(2, sizeof(struct credentials*));
    if(!creds)
      return 0;
    creds[0] = out;
  }else{
    struct credentials *match = 0;
    credentials_get(out, &match);
    if(match){
      // Modify if match found
      strcpy(match->protocol, out->protocol);
      strcpy(match->username, out->username);
      strcpy(match->password, out->password);
      strcpy(match->host, out->host);
      strcpy(match->path, out->path);
    }else{
      // Append otherwise
      size_t count=0;
      for(; creds[count]; ++count);
      creds = realloc(creds, (count+2)*sizeof(struct credentials*));
      if(!creds)
        return 0;
      creds[count] = out;
      creds[count+1] = 0;
    }
  }

  if(!credentials_save(creds)){
    fprintf(stderr, "Failed to save credentials storage.\n");
    return 0;
  }
  
  return 1;
}

int credentials_del(struct credentials *match){
  struct credentials **creds = 0;
  
  if(!credentials_load(&creds)){
    fprintf(stderr, "Failed to load credentials storage.\n");
    return 0;
  }

  for(size_t i=0; creds[i]; ++i){
    struct credentials *cred = creds[i];
    if(FIELDEQ(match, cred, protocol) &&
       FIELDEQ(match, cred, username) &&
       FIELDEQ(match, cred, password) &&
       FIELDEQ(match, cred, host) &&
       FIELDEQ(match, cred, path)){
      size_t j=i+1;
      for(; creds[j]; ++j){
        creds[j-1] = creds[j];
      }
      creds[j] = 0;
      break;
    }
  }

  if(!credentials_save(creds)){
    fprintf(stderr, "Failed to save credentials storage.\n");
    return 0;
  }
  
  return 1;
}

int main(int argc, char **argv){
  struct credentials match = {0};

  if(argc <= 1){
    fprintf(stderr, "Please specify one of: get store erase\n");
    return 1;
  }

  if(!credentials_read(stdin, &match, 0)){
    fprintf(stderr, "Failed to parse credentials spec from standard input.\n");
    return 1;
  }
  
  /* */ if(streq(argv[1], "get")){
    struct credentials *out = 0;
    credentials_get(&match, &out);
    if(!out){
      out = calloc(1, sizeof(struct credentials));
      if(!out)
        return 0;
      strcpy(out->protocol, match.protocol);
      strcpy(out->username, match.username);
      strcpy(out->password, match.password);
      strcpy(out->host, match.host);
      strcpy(out->path, match.path);
      if(!credentials_show_dialog(out))
        return 0;
      credentials_set(out);
    }
    credentials_write(stdout, out, 0);
  }else if(streq(argv[1], "store")){
    credentials_set(&match);
  }else if(streq(argv[1], "erase")){
    credentials_del(&match);
  }else{
    fprintf(stderr, "Invalid operation %s. Must be one of: get store erase\n", argv[1]);
    return 1;
  }
  
  return 0;
}
