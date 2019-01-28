//
// Created by Zer0 on 2019/1/24.
//

#ifndef CTHREADPOOL_THREADPOOL_H
#define CTHREADPOOL_THREADPOOL_H

#include <pthread.h>

typedef enum {
    IDLE,
    BUSY
}THREAD_WORK_STATE_E;

typedef void (*taskFunc)(void *);

/*任务结构体*/
typedef struct {
    taskFunc taskCB;        //任务处理函数
    void *taskP;            //任务处理函数参数
    taskFunc finishedCB;    //任务执行完成的回调
    void *finishedP;        //任务执行完成回调函数的参数
}tpTask_T, *pTPTask_T;

typedef struct qNode_T{
    pTPTask_T task;
    struct qNode_T *next;
}queueNode_T, *pQueueNode_T;

typedef struct{
    pQueueNode_T *head;
    pQueueNode_T *tail;
    pthread_mutex_t lock;
}queue_T, *pQueue_T;

typedef struct {
    THREAD_WORK_STATE_E state;
    pthread_t           tid;
    pthread_mutex_t     lock;
}thread_T, *pThread_T;

typedef struct {
    pThread_T *threads;
    pQueue_T queue;
}threadPool_T, *pThreadPool_T;

pThreadPool_T tpNew(int count);
void tpAddTask(pThreadPool_T tpool, pTPTask_T task);
void tpFree(pThreadPool_T tpool);

#endif //CTHREADPOOL_THREADPOOL_H
