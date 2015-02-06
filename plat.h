#ifndef PLAT_H
#define PLAT_H

// plat.h: ya cross-platform header file.

// uname     gcc:sym   gcc:arch(32,64)              other_arch_syms
// AIX       _AIX      _POWER,_ARCH_PPC             __PPC64__,__powerpc64__,__powerpc
// Darwin    __APPLE__ __x86,__x86_64,universal     powerpc
// HP-UX     __hpux    _LP64
// Linux     linux     __x86,__x86_64
// SunOS     __sun     __sparc,__sparcv9
// (Windows) WIN32     __x86,__x86_64

//  AIX         powerpc,powerpc64
//  Darwin      universal,universal_clang,x86,x8664
//  HP-UX       ia64,ia64_32
//  Linux       ia64,x86,x8664
//  Net2008     32,64
//  Solaris9    sparc,sparc64
//  Solaris     sparc,sparc64,x86,x8664
//  VS2010 32 32_MTDLL
//  VS2010 64 64_MTDLL
//  VS2012 32 32_MTDLL
//  VS2012 64 64_MTDLL
//  VS2013 32 32_MTDLL
//  VS2013 64 64_MTDLL

#define _CRT_SECURE_NO_WARNINGS 1	// WIN32 strerror ...

// warning:unused-result is a GCC irritant. Who cares what sprintf returns? __wur blocks -Werror.
#include <stdint.h>
#undef  __wur
#define __wur

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32

    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <malloc.h>		// Unix alloca declared in <unistd.h>
    #include <process.h>
    #include <tchar.h>
    #include <WinSock2.h>
    #include <WS2tcpip.h>

    #define __LONG_MAX__ LONG_MAX

    #ifndef FD_SETSIZE
        #define FD_SETSIZE 1024
    #endif

    typedef SSIZE_T ssize_t;

    // Mapped functions with the same parameters. MS hates POSIX
    #define bswap_16    _byteswap_ushort
    #define bswap_32    _byteswap_ulong
    #define bswap_64    _byteswap_uint64
    #define getpid      _getpid     //GetCurrentProcessId
    #define poll        WSAPoll
    #define putenv      _putenv
    #define sleep(SECS) Sleep((SECS) * 1000)
    #define strcasecmp  _stricmp
    #define strdup      _strdup
    #define unlink      _unlink

    // Mapped versions of re-entrant functions. The non-_r functions aren't necessary on Windows
    //  because it uses Thread-Local Storage instead.
    #define strtok_r(str, delim, saveptr) strtok(str, delim)
    #define localtime_r(timep, result)    localtime(timep)

    #define BEGIN_ do {
    #define _END __pragma(warning(push)) __pragma(warning(disable:4127)) } while(0) __pragma(warning(pop))

#else // All Unix flavours; assumes gcc

    #include <fcntl.h>
    #ifdef USE_POLL
        #include <poll.h>
    #endif
    #include <pwd.h>
    #include <unistd.h>

    #include <sys/ioctl.h>
    #include <sys/param.h>
    #include <sys/resource.h>
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <sys/time.h>
    #include <sys/wait.h>

    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>

    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET (-1)
    #endif

    #define BEGIN_ do {
    #define _END } while(0)

    #ifdef __APPLE__ // the only BSD we support ...
        static inline uint16_t bswap_16(uint16_t x) { asm("bswap %0" : "=r"(x) : "0"(x)); return x; }
        static inline uint32_t bswap_32(uint32_t x) { asm("bswap %0" : "=r"(x) : "0"(x)); return x; }
        static inline uint64_t bswap_64(uint64_t x) { asm("bswap %0" : "=r"(x) : "0"(x)); return x; }
        // APPLE sys/types.h #includes <machine/endian.h>
    #else
       #include <endian.h>
       #include <byteswap.h>
    #endif

#endif//WIN32

#ifndef ENTER_C
   #ifdef __cplusplus
      #define ENTER_C extern "C" {
      #define LEAVE_C }
   #else
      #define ENTER_C
      #define LEAVE_C
   #endif
#endif//ENTER_C

#endif//PLAT_H
