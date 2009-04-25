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
 * Numery b��d�w zgodne z POSIX.
 *
 * IEEE Std 1003.1-2001, Section 2.3, Error Numbers
 */
///@TODO: Przerobi� na #define
enum POSIX_ERRNO {
    __POSIX_ERRNO_START = 20,
    /// argument list too long
    E2BIG,
    /// permission deined
    EACCESS,
    /// address in use
    EADDRINUSE,
    /// address not available
    EADDRNOTAVAIL,
    /// address family not supported
    EAFNOSUPPORT,
    /// resource unavailable, try again
    EAGAIN,
    /// connection already in progress.
    EALREADY,
    /// bad file descriptor
    EBADF,
    /// bad message
    EBADMSG,
    /// device or resource busy
    EBUSY,
    /// operation canceled
    ECANCELED,
    /// no child processes
    ECHILD,
    /// connection aborted.
    ECONNABORTED,
    /// Connection refused.
    ECONNREFUSED,
    /// Connection reset.
    ECONNRESET,
    /// Resource deadlock would occur.
    EDEADLK,
    /// Destination address required.
    EDESTADDRREQ,
    /// Mathematics argument out of domain of function.
    EDOM,
    /// Reserved.
    EDQUOT,
    /// File exists.
    EEXIST,
    /// Bad address.
    EFAULT,
    /// File too large.
    EFBIG,
    /// Host is unreachable.
    EHOSTUNREACH,
    /// Identifier removed.
    EIDRM,
    /// Illegal byte sequence.
    EILSEQ,
    /// Operation in progress.
    EINPROGRESS,
    /// Interrupted function.
    EINTR,
    /// Invalid argument.
    EINVAL,
    /// I/O error.
    EIO,
    /// Socket is connected.
    EISCONN,
    /// Is a directory.
    EISDIR,
    /// Too many levels of symbolic links.
    ELOOP,
    /// Too many open files.
    EMFILE,
    /// Too many links.
    EMLINK,
    /// Message too large.
    EMSGSIZE,
    /// Reserved.
    EMULTIHOP,
    /// Filename too long.
    ENAMETOOLONG,
    /// Network is down.
    ENETDOWN,
    /// Connection aborted by network.
    ENETRESET,
    /// Network unreachable.
    ENETUNREACH,
    /// Too many files open in system.
    ENFILE,
    /// No buffer space available.
    ENOBUFS,
    /// [XSR] No message is available on the STREAM head read queue. 
    ENODATA,
    /// No such device.
    ENODEV,
    /// No such file or directory.
    ENOENT,
    /// Executable file format error.
    ENOEXEC,
    /// No locks available.
    ENOLCK,
    /// Reserved.
    ENOLINK,
    /// Not enough space.
    ENOMEM,
    /// No message of the desired type.
    ENOMSG,
    /// Protocol not available.
    ENOPROTOOPT,
    /// No space left on device.
    ENOSPC,
    /// [XSR] No STREAM resources. 
    ENOSR,
    /// [XSR]  Not a STREAM. 
    ENOSTR,
    /// Function not supported.
    ENOSYS,
    /// The socket is not connected.
    ENOTCONN,
    /// Not a directory.
    ENOTDIR,
    /// Directory not empty.
    ENOTEMPTY,
    /// Not a socket.
    ENOTSOCK,
    /// Not supported.
    ENOTSUP,
    /// Inappropriate I/O control operation.
    ENOTTY,
    /// No such device or address.
    ENXIO,
    /// Operation not supported on socket.
    EOPNOTSUPP,
    /// Value too large to be stored in data type.
    EOVERFLOW,
    /// Operation not permitted.
    EPERM,
    /// Broken pipe.
    EPIPE,
    /// Protocol error.
    EPROTO,
    /// Protocol not supported.
    EPROTONOSUPPORT,
    /// Protocol wrong type for socket.
    EPROTOTYPE,
    /// Result too large.
    ERANGE,
    /// Read-only file system.
    EROFS,
    /// Invalid seek.
    ESPIPE,
    /// No such process.
    ESRCH,
    /// Reserved.
    ESTALE,
    /// [XSR]  Stream ioctl() timeout. 
    ETIME,
    /// Connection timed out.
    ETIMEDOUT,
    /// Text file busy.
    ETXTBSY,
    /// Operation would block.
    EWOULDBLOCK,
    /// Cross-device link.
    EXDEV,
    __POSIX_ERRNO_MAX
};

/**
 * Numery b��d�w poza standardem.
 */
enum IMAPLA_ERRNO {
    __IMPALA_ERRNO_START = __POSIX_ERRNO_MAX,
    __IMPALA_ERRNO_MAX
};



#endif
