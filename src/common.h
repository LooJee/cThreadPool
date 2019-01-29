//
// Created by Zer0 on 2019/1/28.
//

#ifndef CTHREADPOOL_COMMON_H
#define CTHREADPOOL_COMMON_H

#include <stdlib.h>

#define S_FREE(p) do {  \
    if (p) {            \
        free(p);        \
        p = NULL;       \
    }                   \
} while(0);

#endif //CTHREADPOOL_COMMON_H
