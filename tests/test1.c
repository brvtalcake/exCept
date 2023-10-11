#include <stdio.h>
#include <threads.h>
#include <time.h>
#include <stdlib.h>
#include <stdatomic.h>

#include <exCept.h>

#define DIV_BY_ZERO_EXCEPTION 1

atomic_bool can_start = ATOMIC_VAR_INIT(false);

static inline double divide(int a, int b)
{
    if (b == 0)
        THROW(DIV_BY_ZERO_EXCEPTION, "Division by zero occured in divide() function");
    return (double)a / (double)b;
}

static inline int rand_nb(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

int thread_func(void *arg)
{
    while (!atomic_load(&can_start))
        thrd_yield();
    exC_thrd_setup();
    while (1)
    {
        TRY
        {
            int a = rand_nb(0, 200);
            int b = rand_nb(0, 200);
            fprintf(stderr, "divide(%d, %d) = %f\n", a, b, divide(a, b));
        }
        CATCH(DIV_BY_ZERO_EXCEPTION)
        {
            fprintf(stderr, "Division by zero occured in thread_func() from thread number %d\n", *(int *)arg);
            thrd_exit(DIV_BY_ZERO_EXCEPTION);
        }
        CATCH(e)
        {
            fprintf(stderr, "Unknown exception %d occured in thread_func() from thread number %d\n", e, *(int *)arg);
            thrd_exit(e);
        }
        END_TRY;
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;

    exC_global_setup(42, 0);
    srand(time(NULL));

    thrd_t threads[10];
    int thread_args[10];
    for (int i = 0; i < 10; i++)
    {
        thread_args[i] = i;
        thrd_create(&threads[i], thread_func, &thread_args[i]);
    }
    atomic_store(&can_start, 1);

    for (int i = 0; i < 10; i++)
        thrd_join(threads[i], NULL);

    return 0;
}
