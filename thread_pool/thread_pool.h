#if !defined(__THREADPOOL_H)
#define __THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_THREAD_IN_POOL 200

typedef struct thread_pool_s thread_pool_t;
typedef void (*task_fun)(void *);

typedef enum
{
    NORMAL_PRI = 0,
    EMG_PRI
}pri_flag_t;

/*
** summary             : create and initialize a thread pool
** min_threads_in_pool : min number of threads in pool( > 0 and <= max_threads_in_pool )
** max_threads_in_pool : max number of threads in pool( >= min_threads_in_pool and <= MAXT_IN_POOL )
** task_queue_size     : the size of task queue
** return              : handle of a new thread pool or NULL if error
*/
thread_pool_t *threadpool_create(int min_threads_in_pool, int max_threads_in_pool, int task_queue_size);

/*
** summary     : dispatch a task to thread pool
** pool        : handle of thread pool
** task_here   : the task function to let a thread take
** arg         : argument to task function
** ...         : priority of the task, the default level is NORMAL_PRI if it's not specified
** return      : sequence number of this task in task queue, 0 if the task queue is full
**               the max capability of queue is (min_threads_in_pool + max_threads_in_pool)/2
*/
int dispatch(thread_pool_t *pool, task_fun task_here, void *arg, ...);

/*
** summary           : destroy a thread pool
** pool              : handle of thread pool
** wait_task_flag    : 0 - Cancel all threads immediately ; 1 - inform and wait all threads to exit
** return            : None
*/
void threadpool_destroy(thread_pool_t *pool, int wait_task_flag);

#endif

