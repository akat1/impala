#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

key_t
ftok(const char *path, int id)
{
    ///@todo czekamy na stat
    return id;
}
