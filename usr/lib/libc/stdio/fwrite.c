#include <stdio.h>
#include <unistd.h>

size_t
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    int fd = fileno(stream);
    if(stream->writefn)
        return stream->writefn(stream->cookie, ptr, size*nmemb);
    return write(fd, ptr, size*nmemb);
}
