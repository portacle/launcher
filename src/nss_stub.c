#include <limits.h>
#include <nss.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <stdlib.h>

#define NSS(y) NSS_0(MODULE,y)
#define NSS_0(x,y) NSS_1(x,y)
#define NSS_1(x,y) _nss_##x##y

// Define libnss_dns signatures.
enum nss_status NSS_1(dns, _gethostbyname4_r)(const char *name,
                                              struct gaih_addrtuple **pat,
                                              char *buffer, size_t buflen,
                                              int *errnop, int *h_errnop,
                                              int32_t *ttlp);

enum nss_status NSS_1(dns, _gethostbyname3_r)(const char *name,
                                              int af,
                                              struct hostent *host,
                                              char *buffer, size_t buflen,
                                              int *errnop, int *h_errnop,
                                              int32_t *ttlp,
                                              char **canonp);

enum nss_status NSS_1(dns, _gethostbyname2_r)(const char *name,
                                              int af,
                                              struct hostent *host,
                                              char *buffer, size_t buflen,
                                              int *errnop, int *h_errnop);

enum nss_status NSS_1(dns, _gethostbyname_r)(const char *name,
                                             struct hostent *host,
                                             char *buffer, size_t buflen,
                                             int *errnop, int *h_errnop);

enum nss_status NSS_1(dns, _gethostbyaddr2_r)(const void* addr, socklen_t len,
                                              int af,
                                              struct hostent *host,
                                              char *buffer, size_t buflen,
                                              int *errnop, int *h_errnop,
                                              int32_t *ttlp);

enum nss_status NSS_1(dns, _gethostbyaddr_r)(const void* addr, socklen_t len,
                                             int af,
                                             struct hostent *host,
                                             char *buffer, size_t buflen,
                                             int *errnop, int *h_errnop);

// Define delegating stubs.
enum nss_status NSS(_gethostbyname4_r)(const char *name,
                                       struct gaih_addrtuple **pat,
                                       char *buffer, size_t buflen,
                                       int *errnop, int *h_errnop,
                                       int32_t *ttlp) {
  return _nss_dns_gethostbyname4_r(name, pat, buffer, buflen, errnop, h_errnop, ttlp);
}

enum nss_status NSS(_gethostbyname3_r)(const char *name,
                                       int af,
                                       struct hostent *host,
                                       char *buffer, size_t buflen,
                                       int *errnop, int *h_errnop,
                                       int32_t *ttlp,
                                       char **canonp) {
  return _nss_dns_gethostbyname3_r(name, af, host, buffer, buflen, errnop, h_errnop, ttlp, canonp);
}

enum nss_status NSS(_gethostbyname2_r)(const char *name,
                                       int af,
                                       struct hostent *host,
                                       char *buffer, size_t buflen,
                                       int *errnop, int *h_errnop) {
  return _nss_dns_gethostbyname2_r(name, af, host, buffer, buflen, errnop, h_errnop);
}

enum nss_status NSS(_gethostbyname_r)(const char *name,
                                      struct hostent *host,
                                      char *buffer, size_t buflen,
                                      int *errnop, int *h_errnop) {
  return _nss_dns_gethostbyname_r(name, host, buffer, buflen, errnop, h_errnop);
}

enum nss_status NSS(_gethostbyaddr2_r)(const void* addr, socklen_t len,
                                       int af,
                                       struct hostent *host,
                                       char *buffer, size_t buflen,
                                       int *errnop, int *h_errnop,
                                       int32_t *ttlp) {
  return _nss_dns_gethostbyaddr2_r(addr, len, af, host, buffer, buflen, errnop, h_errnop, ttlp);
}

enum nss_status NSS(_gethostbyaddr_r)(const void* addr, socklen_t len,
                                      int af,
                                      struct hostent *host,
                                      char *buffer, size_t buflen,
                                      int *errnop, int *h_errnop) {
  return _nss_dns_gethostbyaddr_r(addr, len, af, host, buffer, buflen, errnop, h_errnop);
}
