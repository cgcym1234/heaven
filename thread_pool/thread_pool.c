#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>

#include "thread_pool.h"

#define add_node_to_head(node, head) { (node)->next = head; (head) = (node); }
#define add_node_to_tail(node, tail) { (tail)->next = node; (tail) = (node); }
#define exit_if_fail(expr)                             do{ \
            if(expr) { } else{                             \
                fprintf(stderr,                            \
                "file %s: line %d: assertion '%s' failed", \
                __FILE__, __LINE__, #expr);                \
                exit(1);                                   \
            }; }while(0)

#define exit_if_fail_with_message(expr, msg)           do{ \
            if(expr) { } else{                             \
                fprintf(stderr, msg);                      \
                exit(1);                                   \
            }; }while(0)

typedef struct task_node_s task_node_t;
typedef struct task_head_s task_head_t;
typedef struct worker_node_s worker_node_t;
typedef struct worker_head_s worker_head_t;

typedef enum { PRUN, PEXIT } state_t;

struct task_node_s
{
    task_fun fun;           //task function of the queue
    void *arg;              //arguments of the function
    task_node_t *next;
};
struct task_head_s
{
    task_node_t *head;      //the head of the task queue
    task_node_t *tail;      //the tail of the task queue
    task_node_t *recycle;   //the node of the task queue which is allocated memory, when a task is done, the node will be put into this list, for new tasks
    int max_cap;            //the task queue size initialized
    int cur_cap;            //current task queue size allocated
    int avail;              //available task in the queue
};
struct worker_node_s
{
    pthread_t tid;          //worker thead's thead id
    worker_node_t *next;
};
struct worker_head_s
{
    worker_node_t *head;    //the head of worker list
    worker_node_t *recycle; //the node of worker list which is allocated memory, when a worker thread exits, the node will be put into this list, for new workers
    int max;                //the max threads initialized
    int min;                //the min threads initialized
    int cur;                //the number of threads which is working now
};
struct thread_pool_s
{
    struct timeval created_time;
    worker_head_t worker;
    task_head_t task;
    task_head_t emg_task;
    state_t state;

    pthread_mutex_t mutex;
    pthread_cond_t job_posted;
    pthread_cond_t job_taken;
};

static void *do_work(void * );
static void add_worker(thread_pool_t *);
static void del_worker( worker_head_t *, pthread_t);
static int add_task(task_head_t *, task_fun, void *);
static int get_task(task_head_t *, task_fun *, void **);
static void free_task(task_head_t *);
static void free_worker(worker_head_t *);

thread_pool_t *threadpool_create(int min, int max, int qsize)
{
    if(min <= 0 || min > max || max > MAX_THREAD_IN_POOL) return NULL;

    thread_pool_t *pool;
    pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    exit_if_fail_with_message(pool, "threadpool_create() failed, out Of Memory!!!");

    qsize = (qsize > 0 ? qsize : max*100);
    pthread_mutex_init(&(pool->mutex), NULL);
    pthread_cond_init(&(pool->job_posted), NULL);
    pthread_cond_init(&(pool->job_taken), NULL);
    pool->worker.min = min;
    pool->worker.max = max;
    pool->worker.cur = 0;
    pool->worker.head = NULL;
    pool->worker.recycle = NULL;
    /* normal task queue */
    pool->task.max_cap = qsize;
    pool->task.cur_cap = 0;
    pool->task.avail = 0;
    pool->task.head = NULL;
    pool->task.tail = NULL;
    pool->task.recycle = NULL;
    /* emergency task queue */
    pool->emg_task.max_cap = qsize;
    pool->emg_task.cur_cap = 0;
    pool->emg_task.avail = 0;
    pool->emg_task.head = NULL;
    pool->emg_task.tail = NULL;
    pool->emg_task.recycle = NULL;

    pool->state = PRUN;
    gettimeofday(&pool->created_time, NULL);

    int i;
    for(i = 0; i < min; i++) add_worker(pool);

    return pool;
}

static void add_worker(thread_pool_t *pool)
{
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, do_work, (void *)pool);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_create() failed:[%s]\n", strerror(ret));
        return;
    }
    pthread_detach(tid);

    worker_head_t *worker = &pool->worker;
    worker_node_t *temp = NULL;
    if(worker->recycle == NULL)
    {
        worker->recycle = (worker_node_t *)malloc(sizeof(worker_node_t));
        exit_if_fail_with_message(worker->recycle, "add_worker() failed");
        worker->recycle->next = NULL;
    }
    temp = worker->recycle;
    worker->recycle = worker->recycle->next;

    temp->tid = tid;
    add_node_to_head(temp, worker->head);
    worker->cur++;
}

static void del_worker(worker_head_t *worker, pthread_t tid)
{
    register worker_node_t *p1 = NULL;
    register worker_node_t *p2 = NULL;

    if((p1 = p2 = worker->head) == NULL) return;
    if(p1->tid == tid) worker->head = worker->head->next;
    else
    {
        while((p1=p1->next))
        {
            if(p1->tid == tid)
            {
                p2->next = p1->next;
                break;
            }
            p2 = p1;
        }
    }
    if(p1)
    {
        add_node_to_head(p1, worker->recycle);
        if(worker->cur > 0) worker->cur--;
    }
}

static void *do_work(void *owning_pool)
{
    thread_pool_t *pool =(thread_pool_t *)owning_pool;

    state_t mystate = PRUN;
    task_fun mytask = NULL;
    void *myarg = NULL;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cleanup_push((task_fun)pthread_mutex_unlock, (void *)&pool->mutex);

    for( ; ; )
    {
        pthread_mutex_lock(&pool->mutex);
        while((pool->task.head == NULL) && (pool->emg_task.head == NULL))
        {
            if(pool->worker.cur > pool->worker.min || pool->state == PEXIT)
            {
                mystate = PEXIT;
                break;
            }
            pthread_cond_wait(&pool->job_posted, &pool->mutex);
        }

        if(mystate == PEXIT)
        {
            del_worker(&pool->worker, pthread_self());
            break;
        }
        //printf("start get task queue\n");
        if(get_task(&pool->emg_task, &mytask, &myarg) == -1)
        {
            //if emergency queue is null, get normal task
            get_task(&pool->task, &mytask, &myarg);
        }

        pthread_cond_signal(&pool->job_taken);
        pthread_mutex_unlock(&pool->mutex);

        if(mytask)
        {
            mytask(myarg);
            mytask = myarg = NULL;
        }
    }

    pthread_cleanup_pop(1);

    return NULL;
}

int dispatch(thread_pool_t *pool, task_fun task_here, void *arg, ...)
{
    int priority;
    va_list ap;
    va_start(ap, arg);
    priority = va_arg(ap, int);
    va_end(ap);
    priority == EMG_PRI ? 1 : (priority = NORMAL_PRI);

    int rank = 0; char is_add = 1;

    //pthread_cleanup_push((task_fun)pthread_mutex_unlock,(void *) &pool->mutex);
    pthread_mutex_lock(&pool->mutex);

    while(pool->task.recycle == NULL && pool->task.cur_cap >= pool->task.max_cap)
    {
        if(pool->worker.cur < pool->worker.max) add_worker(pool);
        //else { is_add = 0; break; } //Task queue is full, don't wait

        pthread_cond_signal(&pool->job_posted);
        pthread_cond_wait(&pool->job_taken, &pool->mutex);
    }

    if(is_add)
    {
        switch(priority)
        {
            case NORMAL_PRI:
                //printf("add normal task\n");
                rank = add_task(&pool->task, task_here, arg);
                break;
            case EMG_PRI:
                //printf("add emg task\n");
                rank = add_task(&pool->emg_task, task_here, arg);
                break;
        }
        if(rank > 10 && pool->worker.cur < pool->worker.max) add_worker(pool);
    }

    pthread_cond_signal(&pool->job_posted);
    //pthread_cond_broadcast(&pool->job_posted);
    pthread_mutex_unlock(&pool->mutex);

    //pthread_cleanup_pop(0);

    return rank;
}

void threadpool_destroy(thread_pool_t *pool, int flag)
{
    worker_node_t *wp;
    int oldtype;
    printf("cur threads:[%d]\n", pool->worker.cur);

    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);
    pthread_cleanup_push((task_fun)pthread_mutex_unlock, (void *)&pool->mutex);

    if(flag)
    {
        for(wp = pool->worker.head; wp; wp = wp->next) pthread_cancel(wp->tid);
    }else
    {
        pthread_mutex_lock(&pool->mutex);
        pool->state = PEXIT;
        pthread_mutex_unlock(&pool->mutex);

        while(pool->worker.cur)
        {
            pthread_cond_broadcast(&pool->job_posted);
            usleep(100);
        }
    }
    free_worker(&pool->worker);
    free_task(&pool->task);
    free_task(&pool->emg_task);

    pthread_cleanup_pop(0);

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->job_posted);
    pthread_cond_destroy(&pool->job_taken);

    free(pool);
    printf("destroy_threadpool ok\n");
}

static void free_worker(worker_head_t *worker)
{
    worker_node_t *temp;

    while(worker->head || (worker->head = worker->recycle))
    {
        if(worker->head == worker->recycle) worker->recycle = NULL;
        temp = worker->head;
        worker->head = worker->head->next;
        free(temp);
    }
}

static void free_task(task_head_t *task)
{
    task_node_t *temp;

    while(task->head || (task->head = task->recycle))
    {
        if(task->head == task->recycle) task->recycle = NULL;
        temp = task->head;
        task->head = task->head->next;
        free(temp);
    }
}

static int add_task(task_head_t *task, task_fun f1, void *a1)
{
    register task_node_t * temp;

    if(task->recycle == NULL)
    {
        task->recycle = (task_node_t *) malloc(sizeof(task_node_t));
        if(task->recycle == NULL) exit(EXIT_FAILURE);
        task->recycle->next = NULL;
        task->cur_cap++;
    }

    temp = task->recycle;
    task->recycle = temp->next;

    temp->fun = f1;
    temp->arg = a1;
    temp->next = NULL;

    if(task->tail) add_node_to_tail(temp, task->tail)
    else task->head = task->tail = temp;
    //printf("avail task [%d]\n",task->avail);

    return ++task->avail;
}

static int get_task(task_head_t *task, task_fun *f1, void **a1)
{
    register task_node_t *temp;

    if((temp = task->head)==NULL) return -1;
    if(task->head == task->tail) task->head = task->tail = NULL;
    else task->head = temp->next;

    *f1 = temp->fun;
    *a1 = temp->arg;

    add_node_to_head(temp, task->recycle);
    task->avail--;

    return 0;
}

