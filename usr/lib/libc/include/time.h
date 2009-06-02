#ifndef __TIME_H
#define __TIME_H

#include <sys/types.h>
#include <sys/time.h>

int nanosleep(const timespec_t *req, timespec_t *el);

#endif
