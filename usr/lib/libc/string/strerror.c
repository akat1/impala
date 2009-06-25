#include <errno.h>
#include <stdio.h>
#include <string.h>

static char *__err_list[] =
{
"argument list too long",
"permission deined",
"address in use",
"address not available",
"address family not supported",
"resource unavailable try again",
"connection already in progress.",
"bad file descriptor",
"bad message",
"device or resource busy",
"operation canceled",
"no child processes",
"connection aborted.",
"Connection refused.",
"Connection reset.",
"Resource deadlock would occur.",
"Destination address required.",
"Mathematics argument out of domain of function.",
"Reserved.",
"File exists.",
"Bad address.",
"File too large.",
"Host is unreachable.",
"Identifier removed.",
"Illegal byte sequence.",
"Operation in progress.",
"Interrupted function.",
"Invalid argument.",
"I/O error.",
"Socket is connected.",
"Is a directory.",
"Too many levels of symbolic links.",
"Too many open files.",
"Too many links.",
"Message too large.",
"Reserved.",
"Filename too long.",
"Network is down.",
"Connection aborted by network.",
"Network unreachable.",
"Too many files open in system.",
"No buffer space available.",
"[XSR] No message is available on the STREAM head read queue.",
"No such device.",
"No such file or directory.",
"Executable file format error.",
"No locks available.",
"Reserved.",
"Not enough space.",
"No message of the desired type.",
"Protocol not available.",
"No space left on device.",
"[XSR] No STREAM resources.",
"[XSR] Not a STREAM.",
"Function not supported.",
"The socket is not connected.",
"Not a directory.",
"Directory not empty.",
"Not a socket.",
"Not supported.",
"Inappropriate I/O control operation.",
"No such device or address.",
"Operation not supported on socket.",
"Value too large to be stored in data type.",
"Operation not permitted.",
"Broken pipe.",
"Protocol error.",
"Protocol not supported.",
"Protocol wrong type for socket.",
"Result too large.",
"Read-only file system.",
"Invalid seek.",
"No such process.",
"Reserved.",
"[XSR]Stream ioctl() timeout.",
"Connection timed out.",
"Text file busy.",
"Operation would block.",
"Cross-device link."
};

static char *__noerr = "No error.";

char *
strerror(int errnum)
{
    fprintf(stderr, "asking about %u\n", errnum);
    if(errnum == 0)
        return __noerr;
    if(errnum>=E2BIG && errnum<=EXDEV)
        return __err_list[errnum - E2BIG];
    return NULL;
}
