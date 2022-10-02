#ifndef PLATFORM_H
#define PLATFORM_H

// Platform indipendend functions

#include <cstddef>
#include <cstdint>
#include <cstdio>


namespace Platform {
int stricmp (const char *__s1, const char *__s2);
int strnicmp (const char *__s1, const char *__s2, size_t __n);
void strerror (int __errnum, char *__buf, size_t __buflen);

int64_t ftell64 (std::FILE *__stream);
int fseek64 (FILE *__stream, int64_t __off, int __whence);
int fileopen (FILE** out, const char *__restrict __filename,
                const char *__restrict __modes);


}

#endif // PLATFORM_H
