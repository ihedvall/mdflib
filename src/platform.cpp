#include "platform.h"

#include <cstring>

#if (!_MSC_VER)
#include <cerrno>
#include <strings.h>
#endif

namespace Platform {

int stricmp(const char *__s1, const char *__s2) {
#if (_MSC_VER)
  return _stricmp(__s1, __s2);
#else
  return strcasecmp(__s1, __s2);
#endif
}

int strnicmp(const char *__s1, const char *__s2, size_t __n) {
#if (_MSC_VER)
  return _strnicmp(__s1, __s2, __n);
#else
  return strncasecmp(__s1, __s2, __n);
#endif
}

void strerror(int __errnum, char *__buf, size_t __buflen) {
#if (_WIN32)
  strerror_s(__buf, __buflen, __errnum);
#else
  strerror_r(__errnum, __buf, __buflen);
#endif
}

int64_t ftell64(std::FILE *__stream) {
#if (_MSC_VER)
  return _ftelli64(__stream);
#else
  return ftello64(__stream);
#endif
}

int fseek64(std::FILE *__stream, int64_t __off, int __whence) {
#if (_MSC_VER)
  return _fseeki64(__stream, __off, __whence);
#else
  return fseeko64(__stream, __off, __whence);
#endif
}

int fileopen(std::FILE **out, const char *__restrict __filename,
             const char *__restrict __modes) {
  *out = nullptr;
#if (_MSC_VER)
  return fopen_s(out, __filename, __modes);
#else
  *out = fopen(__filename, __modes);
  if (!(*out)) {
    return errno;
  }
  return 0;
#endif
}

} // namespace Platform
