/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski Artur Koninski Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms with or without
 * modification are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT INDIRECT INCIDENTAL SPECIAL EXEMPLARY OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE DATA OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY WHETHER IN CONTRACT STRICT
 * LIABILITY OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */
#ifndef __SYS_ERRNO_H
#define __SYS_ERRNO_H

#define EOK             0  ///< no error

/**
 * Numery b³êdów zgodne z POSIX.
 *
 * IEEE Std 1003.1-2001 Section 2.3 Error Numbers
 */

#define E2BIG           20 ///< argument list too long
#define EACCES          21 ///< permission deined
#define EADDRINUSE      22 ///< address in use
#define EADDRNOTAVAIL   23 ///< address not available
#define EAFNOSUPPORT    24 ///< address family not supported
#define EAGAIN          25 ///< resource unavailable try again
#define EALREADY        26 ///< connection already in progress.
#define EBADF           27 ///< bad file descriptor
#define EBADMSG         28 ///< bad message
#define EBUSY           29 ///< device or resource busy
#define ECANCELED       30 ///< operation canceled
#define ECHILD          31 ///< no child processes
#define ECONNABORTED    32 ///< connection aborted.
#define ECONNREFUSED    33 ///< Connection refused.
#define ECONNRESET      34 ///< Connection reset.
#define EDEADLK         35 ///< Resource deadlock would occur.
#define EDESTADDRREQ    36 ///< Destination address required.
#define EDOM            37 ///< Mathematics argument out of domain of function.
#define EDQUOT          38 ///< Reserved.
#define EEXIST          39 ///< File exists.
#define EFAULT          40 ///< Bad address.
#define EFBIG           41 ///< File too large.
#define EHOSTUNREACH    42 ///< Host is unreachable.
#define EIDRM           43 ///< Identifier removed.
#define EILSEQ          44 ///< Illegal byte sequence.
#define EINPROGRESS     45 ///< Operation in progress.
#define EINTR           46 ///< Interrupted function.
#define EINVAL          47 ///< Invalid argument.
#define EIO             48 ///< I/O error.
#define EISCONN         49 ///< Socket is connected.
#define EISDIR          50 ///< Is a directory.
#define ELOOP           51 ///< Too many levels of symbolic links.
#define EMFILE          52 ///< Too many open files.
#define EMLINK          53 ///< Too many links.
#define EMSGSIZE        54 ///< Message too large.
#define EMULTIHOP       55 ///< Reserved.
#define ENAMETOOLONG    56 ///< Filename too long.
#define ENETDOWN        57 ///< Network is down.
#define ENETRESET       58 ///< Connection aborted by network.
#define ENETUNREACH     59 ///< Network unreachable.
#define ENFILE          60 ///< Too many files open in system.
#define ENOBUFS         61 ///< No buffer space available.
#define ENODATA         62 ///< [XSR] No message is available on the STREAM head read queue.
#define ENODEV          63 ///< No such device.
#define ENOENT          64 ///< No such file or directory.
#define ENOEXEC         65 ///< Executable file format error.
#define ENOLCK          66 ///< No locks available.
#define ENOLINK         67 ///< Reserved.
#define ENOMEM          68 ///< Not enough space.
#define ENOMSG          69 ///< No message of the desired type.
#define ENOPROTOOPT     70 ///< Protocol not available.
#define ENOSPC          71 ///< No space left on device.
#define ENOSR           72 ///< [XSR] No STREAM resources.
#define ENOSTR          73 ///< [XSR] Not a STREAM.
#define ENOSYS          74 ///< Function not supported.
#define ENOTCONN        75 ///< The socket is not connected.
#define ENOTDIR         76 ///< Not a directory.
#define ENOTEMPTY       77 ///< Directory not empty.
#define ENOTSOCK        78 ///< Not a socket.
#define ENOTSUP         79 ///< Not supported.
#define ENOTTY          80 ///< Inappropriate I/O control operation.
#define ENXIO           81 ///< No such device or address.
#define EOPNOTSUPP      82 ///< Operation not supported on socket.
#define EOVERFLOW       83 ///< Value too large to be stored in data type.
#define EPERM           84 ///< Operation not permitted.
#define EPIPE           85 ///< Broken pipe.
#define EPROTO          86 ///< Protocol error.
#define EPROTONOSUPPORT 87 ///< Protocol not supported.
#define EPROTOTYPE      89 ///< Protocol wrong type for socket.
#define ERANGE          90 ///< Result too large.
#define EROFS           91 ///< Read-only file system.
#define ESPIPE          92 ///< Invalid seek.
#define ESRCH           93 ///< No such process.
#define ESTALE          94 ///< Reserved.
#define ETIME           95 ///< [XSR]  Stream ioctl() timeout.
#define ETIMEDOUT       96 ///< Connection timed out.
#define ETXTBSY         97 ///< Text file busy.
#define EWOULDBLOCK     98 ///< Operation would block.
#define EXDEV           99 ///< Cross-device link.

#endif
