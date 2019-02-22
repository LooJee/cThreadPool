//
// Created by Zer0 on 2019/1/24.
//

#include "threadpool.h"
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>

pTPTask_T taskNew(taskFunc task, void *tParam, taskFunc finished, void *fParam)
{
    pTPTask_T t = (pTPTask_T)malloc(sizeof(tpTask_T));
    if (t == NULL) {
        perror("malloc task failed\n");
        return NULL;
    }
    t->taskCB = task;
    t->taskP = tParam;
    t->finishedCB = finished;
    t->finishedP = fParam;

    return t;
}

pQueueNode_T queueNodeNew(pTPTask_T task)
{
    pQueueNode_T qn = (pQueueNode_T)malloc(sizeof(queueNode_T));
    if (qn == NULL) {
        perror("malloc qn failed");
        return NULL;
    }
    qn->task = task;
    qn->next = NULL;

    return qn;
}

void queueNodeFree(pQueueNode_T n)
{
    if (n) {
        S_FREE(n->task);
        free(n);
        n = NULL;
    }
}

pQueue_T queueNew()
{
    pQueue_T q = (pQueue_T)malloc(sizeof(queue_T));
    if (q == NULL) {
        perror("malloc queue failed\n");
        return NULL;
    }

    q->head = NULL;
    q->tail = NULL;

    return q;
}

int queuePush(pQueue_T q, pTPTask_T t)
{
    if (!q) {
        printf("new a queue first\n");
        return -2;
    }

    pQueueNode_T node = queueNodeNew(t);
    if (node == NULL) {
        return -1;
    }

    if (q->head == NULL) {
        q->head = node;
        q->tail = q->head;
    } else {
        q->tail->next = node;
        q->tail = node;
    }

    return 0;
}

pQueueNode_T queuePop(pQueue_T q)
{
    if (!q) {
        printf("new a queue first\n");
        return NULL;
    }

    if (q->head) {
        pQueueNode_T node = q->head;
        q->head = q->head->next;
        return node;
    }

    return NULL;
}

void queueFree(pQueue_T q)
{
    if (q) {
        while (q->head) {
            pQueueNode_T node = queuePop(q);
            queueNodeFree(node);
        }
        S_FREE(q);
    }
}

void *threadFunc(void *arg)
{
    pThreadPool_T tpool = (pThreadPool_T)arg;

    while (tpool->status != READY_EXIT) {
        printf("thread running -- %lu\n", pthread_self());
        pthread_mutex_lock(&tpool->lock);
        while (tpool->queue->head == NULL) {
            pthread_cond_wait(&tpool->cond, &tpool->lock);
        }
        pQueueNode_T n = queuePop(tpool->queue);
        pthread_mutex_unlock(&tpool->lock);
        if (n) {
            if (n->task->taskCB)
                n->task->taskCB(n->task->taskP);

            if (n->task->finishedCB)
                n->task->finishedCB(n->task->finishedP);

            S_FREE(n->task);
            S_FREE(n);
        }
    }

    pthread_exit((void*)0);
}

/*
 * @description : create new thread struct
 */
static pThread_T threadNew(pThreadPool_T tpool)
{
    pThread_T t = (pThread_T)malloc(sizeof(thread_T));
    if (t == NULL) {
        return NULL;
    }
    t->state = IDLE;
    if (pthread_create(&t->tid, NULL, threadFunc, (void*)tpool) != 0) {
        S_FREE(t);
        return NULL;
    }
    return t;
}

/*
 *@description : free thread struct
 */
static void threadFree(pThread_T t)
{
    if (t) {
        void *exitCode;
        pthread_join(t->tid, &exitCode);
        printf("thread : %lu exit with code : %ld\n", (unsigned long)(t->tid), (long)(exitCode));
        S_FREE(t);
    }
}

/*
 * @description : create thread pools according to size
 */
pThreadPool_T tpNew(int size)
{
    pThreadPool_T tpool = (pThreadPool_T)malloc(sizeof(threadPool_T));
    if (tpool == NULL) {
        perror("malloc thread pool failed\n");
        return NULL;
    }
    //init task queue
    tpool->queue = queueNew();
    if (tpool->queue == NULL) {
        perror("malloc queue failed\n");
        tpFree(tpool);
        return NULL;
    }
    tpool->status = IDLE;

    //init threads
    tpool->threads = (pThread_T*)malloc(sizeof(pThread_T*)*size);
    if (tpool->threads == NULL) {
        perror("malloc threads failed\n");
        S_FREE(tpool);
        return NULL;
    }
    tpool->size = 0;

    //init lock
    if (pthread_mutex_init(&(tpool->lock), NULL) != 0) {
        tpFree(tpool);
        return NULL;
    }

    if (pthread_cond_init(&(tpool->cond), NULL) != 0) {
        tpFree(tpool);
        return NULL;
    }

    for (int i = 0; i != size; ++i) {
        tpool->threads[i] = threadNew(tpool);
        if (tpool->threads[i] == NULL) {
            printf("malloc thread : %d failed\n", i);
            tpFree(tpool);
            return NULL;
        }
        printf("thread : %lu created\n", (unsigned long)(tpool->threads[i]->tid));
        ++tpool->size;
    }

    return tpool;
}

/*
 * @descripton : add task to the task queue of specific thread pool
 */
void tpAddTask(pThreadPool_T tpool, pTPTask_T task)
{
    pthread_mutex_lock(&(tpool->lock));
    queuePush(tpool->queue, task);
    pthread_cond_broadcast(&tpool->cond);
    pthread_mutex_unlock(&(tpool->lock));
}

/*
 * @description : free the specific thread pool
 */
void tpFree(pThreadPool_T tpool)
{
    if (!tpool) {
        return;
    }

    tpool->status = READY_EXIT;

    pthread_cond_broadcast(&tpool->cond);
    //free threads
    for (int i = 0; i != tpool->size; ++i) {
        if (tpool->threads[i]) {
            threadFree(tpool->threads[i]);
        }
    }

    //free queue
    pthread_mutex_lock(&(tpool->lock));
    queueFree(tpool->queue);
    pthread_mutex_unlock(&(tpool->lock));

    //free lock
    pthread_mutex_destroy(&(tpool->lock));
    pthread_cond_destroy(&(tpool->cond));
}

