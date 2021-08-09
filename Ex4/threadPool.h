//Yahav Zarfati

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"

typedef struct thread_pool
{
    OSQueue* tasksQueue;
    pthread_t* pthreadArr;
    int stop;
    int numOfThreads;
    int count;
    pthread_mutex_t tpMutex;
    pthread_mutex_t tpDestroyMutex;
    pthread_cond_t cond;
}ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
