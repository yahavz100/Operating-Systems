//Yahav Zarfati

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "threadPool.h"

#define ERROR -1

typedef struct task
{
    void (*functionPointer)(void*);
    void* parameter;
}Task;

/*Function is being sent to threads on creation,
 * its role is to keep the threads in the given thread pool alive and busy
 * by executing tasks belong to thread pool queue
 * returns (NULL)
 */
void *executeTasks(void *threadPool) {
    //Check if invalid thread pool
    if(threadPool == NULL) {
        return (NULL);
    }
    ThreadPool *tp = (ThreadPool*) threadPool;

    //Reading shared memory - mutex, threads continue to run tasks until stop is on and no tasks
    while(1) {
        if(pthread_mutex_lock(&tp->tpMutex) != 0 ) {
            perror("executeTasks:pthread_mutex_lock failed");
            tpDestroy(tp, 0);
            exit(ERROR);
        }

        //If stop is off and no tasks to execute, hold threads
        while(!tp->stop && tp->count == 0) {
            if(pthread_cond_wait(&tp->cond, &tp->tpMutex) != 0) {
                perror("executeTasks:pthread_cond_wait failed");
                tpDestroy(tp, 0);
                exit(ERROR);
            }
        }
        //If stop is on and no tasks to execute
        if(tp->stop && tp->count == 0)
            break;

        //Invoke stored task and parameter
        Task *task = osDequeue(tp->tasksQueue);
        tp->count--;
        void (*funcPtr)(void *) = task->functionPointer;
        void *parameter = task->parameter;
        pthread_mutex_unlock(&tp->tpMutex);
        (*funcPtr)(parameter);
        free(task);

    }
    if(pthread_mutex_unlock(&tp->tpMutex) != 0 ) {
        perror("executeTasks:pthread_mutex_unlock failed");
        tpDestroy(tp, 0);
        exit(ERROR);
    }
    pthread_exit(NULL);
    return (NULL);
}

/* Function create thread pool with give number of threads,
 * returns a pointer to one created
 */
ThreadPool* tpCreate(int numOfThreads) {
    //Check for invalid input
    if (numOfThreads <= 0) {
        perror("tpCreate:Invalid number of threads");
        exit(ERROR);
    }

    //Allocate new memory in heap
    ThreadPool *tp = (ThreadPool*) malloc(sizeof(ThreadPool));

    //Check if function success
    if (tp == NULL) {
        perror("tpCreate:malloc failed");
        exit(ERROR);
    }

    //Create array of threads in heap
    tp->pthreadArr = (pthread_t*) malloc(sizeof(pthread_t) * numOfThreads);
    //Check if function success
    if (tp->pthreadArr == NULL) {
        free(tp);
        perror("tpCreate:malloc failed");
        exit(ERROR);
    }

    //Set default values for struct members
    tp->stop = 0;
    tp->numOfThreads = numOfThreads;
    tp->count = 0;
    tp->tasksQueue = osCreateQueue();

    //Check if function success
    if (pthread_mutex_init(&tp->tpMutex, NULL) != 0) {
        perror("tpCreate:pthread_mutex_init failed");
        osDestroyQueue(tp->tasksQueue);
        free(tp->pthreadArr);
        free(tp);
        exit(ERROR);
    }

    //Check if function success
    if (pthread_mutex_init(&tp->tpDestroyMutex, NULL) != 0) {
        perror("tpCreate:pthread_mutex_init failed");
        if(pthread_mutex_destroy(&(tp->tpMutex)) != 0) {
            perror("tpCreate:pthread_mutex_destroy failed");
        }
        osDestroyQueue(tp->tasksQueue);
        free(tp->pthreadArr);
        free(tp);
        exit(ERROR);
    }

    //Create threads according to input, each run executeTask function,
    // with this thread pool as argument, store each one tid in threads array
    int i;
    for(i = 0; i < numOfThreads; i++) {
        //Check if function success
        if (pthread_create(&tp->pthreadArr[i], NULL, executeTasks, (void*) tp) != 0) {
            perror("tpCreate:pthread_create failed");
            tp->numOfThreads = i;
            tpDestroy(tp, 0);
            exit(ERROR);
        }
    }
    return tp;
}

/* Function destroys given thread pool, free all memory,
 * in case of parameter = 0, removes waiting tasks to be executed
 * else, waiting for all tasks to finish then destroys the thread pool
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    //Check if given pointer to thread pool invalid
    if (threadPool == NULL) {
        perror("tpDestroy:Invalid threadPool");
        exit(ERROR);
    }

    //Turn on stop flag, signal to all threads to continue last tasks
    if(pthread_mutex_lock(&threadPool->tpDestroyMutex) != 0) {
        perror("tpDestroy:pthread_mutex_lock failed");
        if(pthread_mutex_destroy(&(threadPool->tpMutex)) != 0) {
            perror("tpDestroy:pthread_mutex_destroy failed");
        }
        if(pthread_mutex_destroy(&(threadPool->tpDestroyMutex)) != 0) {
            perror("tpDestroy:pthread_mutex_destroy failed");
        }
        if(pthread_cond_destroy(&(threadPool->cond)) != 0) {
            perror("tpDestroy:pthread_cond_destroy failed");
        }
        osDestroyQueue(threadPool->tasksQueue);
        free(threadPool->pthreadArr);
        free(threadPool);
        exit(ERROR);
    }
    threadPool->stop = 1;

    //Signal to threads to finish immediately if should not wait for tasks
    if(shouldWaitForTasks == 0)
        threadPool->count = 0;
    if(pthread_cond_broadcast(&threadPool->cond) != 0) {
        perror("tpDestroy:pthread_cond_broadcast failed");
    }
    if(pthread_mutex_unlock(&threadPool->tpDestroyMutex) != 0) {
        perror("tpDestroy:pthread_mutex_unlock failed");
    }

    //Wait for all threads to join
    int i;
    for (i = 0; i < threadPool->numOfThreads; i++) {
        if (pthread_join(threadPool->pthreadArr[i], NULL) != 0) {
            osDestroyQueue(threadPool->tasksQueue);
            free(threadPool->pthreadArr);
            free(threadPool);
            perror("tpDestroy:pthread_join failed");
            exit(ERROR);
        }
    }

    //Destroy all fields of thread pool and free memory
    free(threadPool->pthreadArr);
    osDestroyQueue(threadPool->tasksQueue);
    //pthread_mutex_lock(&threadPool->tpDestroyMutex);
    if(pthread_mutex_destroy(&(threadPool->tpMutex)) != 0) {
        perror("tpDestroy:pthread_mutex_destroy failed");
    }
    if(pthread_mutex_destroy(&(threadPool->tpDestroyMutex)) != 0) {
        perror("tpDestroy:pthread_mutex_destroy failed");
    }
    if(pthread_cond_destroy(&(threadPool->cond)) != 0) {
        perror("tpDestroy:pthread_cond_destroy failed");
    }
    free(threadPool);
}

/* Function insert to given thread pool a task,
 * task->head = OSNode pointer, pointer to function pointer
 * task->tail = OSNode pointer, pointer to parameter for function
 * if tpDestroy was called returns -1, otherwise 0
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    //Check for invalid thread or function pointer
    if(threadPool == NULL || computeFunc == NULL)
        return 0;

    //Check if tpDestroy was called
    pthread_mutex_lock(&threadPool->tpMutex);
    if(threadPool->stop)
        return -1;
    pthread_mutex_unlock(&threadPool->tpMutex);

    //Insert task into queue of thread pool
    Task* task = (Task*) malloc(sizeof(Task));
    task->functionPointer = computeFunc;
    task->parameter = param;

    pthread_mutex_lock(&threadPool->tpMutex);
    osEnqueue(threadPool->tasksQueue, task);
    threadPool->count++;

    //Signal to threads a task is in queue to be executed
    if(pthread_cond_signal(&threadPool->cond) != 0) {
        perror("tpInsertTask:pthread_cond_signal failed");
        tpDestroy(threadPool, 0);
        exit(ERROR);
    }
    pthread_mutex_unlock(&threadPool->tpMutex);
    return 0;
}
