/*
 * example_mutex.c
 */
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static DEFINE_MUTEX(mymutex);

static struct task_struct *thread_1;
static struct task_struct *thread_2;
static int shared_variable = 0;

/* Thread function that will acquire the mutex, perform some work, and release the mutex */
static int thread_function(void *data)
{
    const char *thread_num = (const char *)data;

    pr_info("%s started, trying to acquire the mutex...\n", thread_num);

    /* Try to lock the mutex */
    if (mutex_lock_interruptible(&mymutex) != 0) {
        pr_info("%s failed to acquire the mutex.\n", thread_num);
        return -1;
    }

    pr_info("%s acquired the lock\n", thread_num);

    /* Modify the shared variable */
    for (int i = 0; i < 5; i++) {
        pr_info("%s working on the shared variable, current value: %d\n", thread_num, shared_variable);
        shared_variable += 1; 
        msleep(500);     
    }

    pr_info("%s releasing the mutex, final value of the shared variable: %d\n", thread_num, shared_variable);

    /* Unlock the mutex */
    mutex_unlock(&mymutex);

    pr_info("%s finished work and released the mutex.\n", thread_num);

    return 0;
}

static int __init example_mutex_init(void)
{

    pr_info("example_mutex init\n");

    thread_1 = kthread_run(thread_function, "Thread 1", "thread_1");

    thread_2 = kthread_run(thread_function, "Thread 2", "thread_2");

    return 0;
}

static void __exit example_mutex_exit(void)
{
    pr_info("example_mutex exit\n");

    kthread_stop(thread_1);
    kthread_stop(thread_2);
}

module_init(example_mutex_init);
module_exit(example_mutex_exit);

MODULE_DESCRIPTION("Mutex example");
MODULE_LICENSE("GPL");
