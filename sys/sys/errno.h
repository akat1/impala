/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */
#ifndef __SYS_ERRNO_H
#define __SYS_ERRNO_H

/**
 * Numery b³êdów zgodne z POSIX.
 *
 * IEEE Std 1003.1-2001, Section 2.3, Error Numbers
 */
///@todo Przerobiæ na #define
enum POSIX_ERRNO {
    __POSIX_ERRNO_START = 20,
    E2BIG,       ///< argument list too long
    EACCESS,     ///< permission deined
    EADDRINUSE,  ///< address in use
    EADDRNOTAVAIL,///< address not available
    EAFNOSUPPORT,///< address family not supported
    EAGAIN,      ///< resource unavailable, try again
    EALREADY,    ///< connection already in progress.
    EBADF,       ///< bad file descriptor
    EBADMSG,     ///< bad message
    EBUSY,       ///< device or resource busy
    ECANCELED,   ///< operation canceled
    ECHILD,      ///< no child processes
    ECONNABORTED,///< connection aborted.
    ECONNREFUSED,///< Connection refused.
    ECONNRESET,  ///< Connection reset.
    EDEADLK,     ///< Resource deadlock would occur.
    EDESTADDRREQ,///< Destination address required.
    EDOM,        ///< Mathematics argument out of domain of function.
    EDQUOT,      ///< Reserved.
    EEXIST,      ///< File exists.
    EFAULT,      ///< Bad address.
    EFBIG,       ///< File too large.
    EHOSTUNREACH,///< Host is unreachable.
    EIDRM,       ///< Identifier removed.
    EILSEQ,      ///< Illegal byte sequence.
    EINPROGRESS, ///< Operation in progress.
    EINTR,       ///< Interrupted function.
    EINVAL,      ///< Invalid argument.
    EIO,         ///< I/O error.
    EISCONN,     ///< Socket is connected.
    EISDIR,      ///< Is a directory.
    ELOOP,       ///< Too many levels of symbolic links.
    EMFILE,      ///< Too many open files.
    EMLINK,      ///< Too many links.
    EMSGSIZE,    ///< Message too large.
    EMULTIHOP,   ///< Reserved.
    ENAMETOOLONG,///< Filename too long.
    ENETDOWN,    ///< Network is down.
    ENETRESET,   ///< Connection aborted by network.
    ENETUNREACH, ///< Network unreachable.
    ENFILE,      ///< Too many files open in system.
    ENOBUFS,     ///< No buffer space available.
    ENODATA,     ///< [XSR] No message is available on the STREAM head read queue. 
    ENODEV,      ///< No such device.
    ENOENT,      ///< No such file or directory.
    ENOEXEC,     ///< Executable file format error.
    ENOLCK,      ///< No locks available.
    ENOLINK,     ///< Reserved.
    ENOMEM,      ///< Not enough space.
    ENOMSG,      ///< No message of the desired type.
    ENOPROTOOPT, ///< Protocol not available.
    ENOSPC,      ///< No space left on device.
    ENOSR,       ///< [XSR] No STREAM resources.
    ENOSTR,      ///< [XSR]  Not a STREAM. 
    ENOSYS,      ///< Function not supported.
    ENOTCONN,    ///< The socket is not connected.
    ENOTDIR,     ///< Not a directory.
    ENOTEMPTY,   ///< Directory not empty.
    ENOTSOCK,    ///< Not a socket.
    ENOTSUP,     ///< Not supported.
    ENOTTY,      ///< Inappropriate I/O control operation.
    ENXIO,       ///< No such device or address.
    EOPNOTSUPP,  ///< Operation not supported on socket.
    EOVERFLOW,   ///< Value too large to be stored in data type.
    EPERM,       ///< Operation not permitted.
    EPIPE,       ///< Broken pipe.
    EPROTO,      ///< Protocol error.
    EPROTONOSUPPORT,///< Protocol not supported.
    EPROTOTYPE,  ///< Protocol wrong type for socket.
    ERANGE,      ///< Result too large.
    EROFS,       ///< Read-only file system.
    ESPIPE,      ///< Invalid seek.
    ESRCH,       ///< No such process.
    ESTALE,      ///< Reserved.
    ETIME,       ///< [XSR]  Stream ioctl() timeout. 
    ETIMEDOUT,   ///< Connection timed out.
    ETXTBSY,     ///< Text file busy.
    EWOULDBLOCK, ///< Operation would block.
    EXDEV,       ///< Cross-device link.
    __POSIX_ERRNO_MAX
};

/**
 * Numery b³êdów poza standardem.
 */
enum IMAPLA_ERRNO {
    __IMPALA_ERRNO_START = __POSIX_ERRNO_MAX,
    __IMPALA_ERRNO_MAX
};



#endif
