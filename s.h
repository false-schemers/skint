/* s.h -- system-dependent stuff */

#if defined(__GNUC__) && defined(__linux)
  #ifdef _FEATURES_H
    #warning too late to select features
  #endif
  #if !defined( _GNU_SOURCE )
    #define _GNU_SOURCE
  #endif
  #define _ISOC99_SOURCE
  #define _XOPEN_SOURCE 500
  #if defined( __INTERIX )
    #define _XOPEN_SOURCE_EXTENDED 1
  #endif
#endif

/* this is for MS headers; shouldn't affect others */
#define _CRT_SECURE_NO_WARNINGS 1
  
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <locale.h>
#include <stdarg.h>


#if defined(_MSC_VER)
#ifdef _POSIX_
#undef _POSIX_
#endif
#if (_WIN32_WINNT < 0x0500)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <fcntl.h>
#include <io.h>
#include <share.h> /* for _SH_DENYNO */
#include <direct.h>
#include <sys/types.h>
#include <process.h> /* for getpid */
#include <locale.h> /* for setlocale */
#include <sys/stat.h> /* for _S_IREAD|_S_IWRITE */
#include <windows.h>
#if _MSC_VER >= 1600
#include <crtversion.h>
#endif
#if defined(_VC_CRT_MAJOR_VERSION) && (_VC_CRT_MAJOR_VERSION >= 14)
void __cdecl __acrt_errno_map_os_error(unsigned long const oserrno);
#define _dosmaperr(oserrno) __acrt_errno_map_os_error(oserrno)
#else
_CRTIMP void __cdecl _dosmaperr(unsigned long);
#endif
#define getcwd _getcwd
#define chdir _chdir
#define mkdir(d,m) _mkdir(d)
#define rmdir _rmdir
#define stat(a,b) _stat(a,b)
#define isatty _isatty
#define fileno _fileno
#define setbmode(fp) _setmode(_fileno(fp),_O_BINARY)
typedef struct _stat stat_t;
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#ifndef EEXIST 
#include <errno.h>
#endif
#elif defined(__GNUC__) && defined(__APPLE__) 
#include <mach-o/dyld.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#define setbmode(fp) /* nothing */
typedef struct stat stat_t; 
#elif defined(__GNUC__) && defined(__linux)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#define setbmode(fp) /* nothing */
#ifdef _LFS64_LARGEFILE
#define stat(a,b) stat64(a,b)
typedef struct stat64 stat_t; 
#else
typedef struct stat stat_t; 
#endif
#elif defined(__GNUC__) && (defined(_WIN32) || defined(__INTERIX))
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#define setbmode(fp) /* nothing */
#ifdef _LFS64_LARGEFILE
#define stat(a,b) stat64(a,b)
typedef struct stat64 stat_t; 
#else
typedef struct stat stat_t; 
#endif
#if defined(__INTERIX)
#include <signal.h>
#include <strings.h>
#endif
#elif defined(__SVR4) && defined(__sun)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#define setbmode(fp) /* nothing */
#ifdef _LFS64_LARGEFILE
#define stat(a,b) stat64(a,b)
typedef struct stat64 stat_t; 
#else
typedef struct stat stat_t; 
#endif
#include <libgen.h>
#include <signal.h>
#else
#error "s.h: add your platform here" 
#endif
