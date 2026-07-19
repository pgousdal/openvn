#ifndef OPENVN_TEST_CHECK_H
#define OPENVN_TEST_CHECK_H

#include <stdio.h>
#include <stdlib.h>

#define OPENVN_TEST_CHECK(expression)                                      \
    do {                                                                   \
        if (!(expression)) {                                               \
            fprintf(                                                       \
                stderr,                                                    \
                "%s:%d: check failed: %s\n",                             \
                __FILE__,                                                  \
                __LINE__,                                                  \
                #expression                                                \
            );                                                             \
            abort();                                                       \
        }                                                                  \
    } while (0)

#endif
