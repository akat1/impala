#ifndef __ASSERT_H
#define __ASSERT_H

#define assert(expr)\
    if (!(expr)) {\
        fprintf(stderr,"Assertion failed (%s) at %s:%u", #expr, \
            __FILE__, __LINE__);\
    }

#endif
