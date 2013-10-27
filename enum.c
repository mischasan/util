// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 2 as
// published by the Free Software Foundation.  You may not use, modify or
// distribute this program under any other version of the GNU General
// Public License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// IF YOU ARE UNABLE TO WORK WITH GPL2, CONTACT ME.
//-------------------------------------------------------------------

#include "msutil.h"
#include <netdb.h>          // EAI_*

char const *errname[] = {
    /*_00*/ "",              "EPERM",           "ENOENT",       "ESRCH",           "EINTR",
    /*_05*/ "EIO",           "ENXIO",           "E2BIG",        "ENOEXEC",         "EBADF",
    /*_10*/ "ECHILD",        "EAGAIN",          "ENOMEM",       "EACCES",          "EFAULT",
    /*_15*/ "ENOTBLK",       "EBUSY",           "EEXIST",       "EXDEV",           "ENODEV",
    /*_20*/ "ENOTDIR",       "EISDIR",          "EINVAL",       "ENFILE",          "EMFILE",
    /*_25*/ "ENOTTY",        "ETXTBSY",         "EFBIG",        "ENOSPC",          "ESPIPE",
    /*_30*/ "EROFS",         "EMLINK",          "EPIPE",        "EDOM",            "ERANGE",
#if   defined(__FreeBSD__)
    /*_35*/ "EDEADLK",       "EINPROGRESS",     "EALREADY",     "ENOTSOCK",        "EDESTADDRREQ",
    /*_40*/ "EMSGSIZE",      "EPROTOTYPE",      "ENOPROTOOPT",  "EPROTONOSUPPORT", "ESOCKTNOSUPPORT",
    /*_45*/ "EOPNOTSUPP",    "EPFNOSUPPORT",    "EAFNOSUPPORT", "EADDRINUSE",      "EADDRNOTAVAIL",
    /*_50*/ "ENETDOWN",      "ENETUNREACH",     "ENETRESET",    "ECONNABORTED",    "ECONNRESET",
    /*_55*/ "ENOBUFS",       "EISCONN",         "ENOTCONN",     "ESHUTDOWN",       "ETOOMANYREFS",
    /*_60*/ "ETIMEDOUT",     "ECONNREFUSED",    "ELOOP",        "ENAMETOOLONG",    "EHOSTDOWN",
    /*_65*/ "EHOSTUNREACH",  "ENOTEMPTY",       "EPROCLIM",     "EUSERS",          "EDQUOT",
    /*_70*/ "ESTALE",        "EREMOTE",         "EBADRPC",      "ERPCMISMATCH",    "EPROGUNAVAIL",
    /*_75*/ "EPROGMISMATCH", "EPROCUNAVAIL",    "ENOLCK",       "ENOSYS",          "EFTYPE",
    /*_80*/ "EAUTH",         "ENEEDAUTH",       "EIDRM",        "ENOMSG",          "EOVERFLOW",
    /*_85*/ "ECANCELED",     "EILSEQ",          "ENOATTR",      "EDOOFUS",         "EBADMSG",
    /*_90*/ "EMULTIHOP",     "ENOLINK",         "EPROTO"                           
#elif defined(__linux__)
    /*_35*/ "EDEADLK",       "ENAMETOOLONG",    "ENOLCK",       "ENOSYS",          "ENOTEMPTY",
    /*_40*/ "ELOOP",         "E041",            "ENOMSG",       "EIDRM",           "ECHRNG",
    /*_45*/ "EL2NSYNC",      "EL3HLT",          "EL3RST",       "ELNRNG",          "EUNATCH",
    /*_50*/ "ENOCSI",        "EL2HLT",          "EBADE",        "EBADR",           "EXFULL",
    /*_55*/ "ENOANO",        "EBADRQC",         "EBADSLT",      "E058",            "EBFONT",
    /*_60*/ "ENOSTR",        "ENODATA",         "ETIME",        "ENOSR",           "ENONET",
    /*_65*/ "ENOPKG",        "EREMOTE",         "ENOLINK",      "EADV",            "ESRMNT",
    /*_70*/ "ECOMM",         "EPROTO",          "EMULTIHOP",    "EDOTDOT",         "EBADMSG",
    /*_75*/ "EOVERFLOW",     "ENOTUNIQ",        "EBADFD",       "EREMCHG",         "ELIBACC",
    /*_80*/ "ELIBBAD",       "ELIBSCN",         "ELIBMAX",      "ELIBEXEC",        "EILSEQ",
    /*_85*/ "ERESTART",      "ESTRPIPE",        "EUSERS",       "ENOTSOCK",        "EDESTADDRREQ",
    /*_90*/ "EMSGSIZE",      "EPROTOTYPE",      "ENOPROTOOPT",  "EPROTONOSUPPORT", "ESOCKTNOSUPPORT",
    /*_95*/ "EOPNOTSUPP",    "EPFNOSUPPORT",    "EAFNOSUPPORT", "EADDRINUSE",      "EADDRNOTAVAIL",
    /*100*/ "ENETDOWN",      "ENETUNREACH",     "ENETRESET",    "ECONNABORTED",    "ECONNRESET",
    /*105*/ "ENOBUFS",       "EISCONN",         "ENOTCONN",     "ESHUTDOWN",       "ETOOMANYREFS",
    /*110*/ "ETIMEDOUT",     "ECONNREFUSED",    "EHOSTDOWN",    "EHOSTUNREACH",    "EALREADY",
    /*115*/ "EINPROGRESS",   "ESTALE",          "EUCLEAN",      "ENOTNAM",         "ENAVAIL",
    /*120*/ "EISNAM",        "EREMOTEIO",       "EDQUOT",       "ENOMEDIUM",       "EMEDIUMTYPE",
    /*125*/ "ECANCELED",     "ENOKEY",          "EKEYEXPIRED",  "EKEYREVOKED",     "EKEYREJECTED",
    /*130*/ "EOWNERDEAD",    "ENOTRECOVERABLE", "ERFKILL"                          
#endif
};

int const nerrnames = sizeof(errname)/sizeof(*errname);

char const *signame[] = {
    /*00*/ "",        "SIGHUP",    "SIGINT",    "SIGQUIT",  "SIGILL",
    /*05*/ "SIGTRAP", "SIGABRT",   "SIGBUS",    "SIGFPE",   "SIGKILL",
    /*10*/ "SIGUSR1", "SIGSEGV",   "SIGUSR2",   "SIGPIPE",  "SIGALRM",
    /*15*/ "SIGTERM", "SIGSTKFLT", "SIGCHLD",   "SIGCONT",  "SIGSTOP",
    /*20*/ "SIGTSTP", "SIGTTIN",   "SIGTTOU",   "SIGURG",   "SIGXCPU",
    /*25*/ "SIGXFSZ", "SIGVTALRM", "SIGPROF",   "SIGWINCH", "SIGIO",
    /*30*/ "SIGPWR",  "SIGSYS",    "SIGUNUSED",             
};
int const nsignames = sizeof(signame)/sizeof(*signame);

// getaddrinfo returns 0 (success) or a negative error code (-1..-11)
char const *eainame[] = {
    //     0    -1              -2            -3           -4          -5           -6            7          8           9
    "",                 // 0
    "EAI_BADFLAGS",     // -1
    "EAI_NONAME",       // -2
    "EAI_AGAIN",        // -3
    "EAI_FAIL",         // -4
    "EAI_NODATA",       // -5
    "EAI_FAMILY",       // -6
    "EAI_SOCKTYPE",     // -7
    "EAI_SERVICE",      // -8
    "EAI_ADDRFAMILY",   // -9
    "EAI_MEMORY",       // -10
    "EAI_SYSTEM"        // -11
};

int const neainames = sizeof(eainame)/sizeof(*eainame);
