#ifndef __SRC_MACRO_H__
#define __SRC_MACRO_H__

#include <string.h>
#include <assert.h>
#include "util.h"

#define WEBSERVER_ASSERT(x)                                                                                            \
    if (!x) {                                                                                                          \
        WEBSERVER_LOG_ERROR(WEBSERVER_LOG_ROOT())                                                                      \
            << "ASSERTION: " #x << "\nbacktrace:\n"                                                                    \
            << webserver::BacktraceToString(100, 2, "      ");                                                         \
        assert(x);                                                                                                     \
    }

#define WEBSERVER_ASSERT2(x, w)                                                                                        \
    if (!(x)) {                                                                                                        \
        WEBSERVER_LOG_ERROR(WEBSERVER_LOG_ROOT()) << "ASSERTION: " #x << "\n"                                          \
            << w << "\nbacktrace:\n"                                                                                   \
            << webserver::BacktraceToString(100, 2, "    ");                                                           \
        assert(x);                                                                                                     \
    }

#endif