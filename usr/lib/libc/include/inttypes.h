#ifndef __INTTYPES_H
#define __INTTYPES_H

#include <stdint.h>

#define PRIdMAX "ll" "d"

intmax_t strtoimax(const char *nptr, char **endptr, int base);

#endif
