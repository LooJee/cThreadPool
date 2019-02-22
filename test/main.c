//
// Created by Zer0 on 2019/1/29.
//

#include <stdio.h>
#include <unistd.h>
#include "../src/threadpool.h"

void tfunc(void *param)
{
//    int i = (int)param;
    for (int i = 0; i < 10; ++i) {
        printf("param : %d, i = %d from thread -- %lu\n", (int)param, i, (unsigned long)pthread_self());
        sleep(1);
    }
//    sleep(2);
}

int main(int argc, char *argv[])
{
    pThreadPool_T pool = tpNew(4);

    for (int i = 0; i < 100; ++i) {
        taskFunc task = tfunc;
        pTPTask_T t = taskNew(task, (void*)i, NULL, NULL);
        tpAddTask(pool, t);
    }

    getchar();

    tpFree(pool);

    return 0;
}
