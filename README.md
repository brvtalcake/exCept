# exCept

exCept is a two files C "library" providing multithread-aware exception handling in pure C. To see all the provided macros and functions, see ["Index of available macros"](#index-of-available-macros) and ["Index of available functions"](#index-of-available-functions), respectively.

> **Note**
>
> `FINALLY` is not yet supported, and probably won't ever be supported.
> However, if you want a (non-portable - only GCC for now) `DEFER` macro (that could "imitate" a `FINALLY`), check [this little repo](https://github.com/brvtalcake/defer) of mine.

## Usage

First thing you need to do is to set the size of the jmp_buf stack (a kind of exception stack, in our case, or even the amount of `TRY / CATCH` statements you can nest), with :

```c
exC_global_setup(
    42, // The aformentioned size
    0 // Potential flags (none are implemented to this point, but could be in the future)
);
```

Then, in each thread, simply call ...

```c
exC_thrd_setup();
```

... and you can start using the provided macros as you would use them in higher level programming languages. Here is a basic example of use :

```c
/* If you want to use lowercase keywords (at your own risk), then uncomment the following line */
/* (it must appear before including header, and you can not use both syntaxes in the same time) */

/* #define EXCEPT_LOWERCASE */

#include <stdio.h>
#include <stdlib.h>
#include "exCept.h"

void first_test_func(void)
{
    fprintf(stderr, "first_test_func\n");
    THROW(3);
    fprintf(stderr, "first_test_func (unreachable)\n");
}

void second_test_func(void)
{
    fprintf(stderr, "second_test_func\n");
    first_test_func();
    fprintf(stderr, "second_test_func (unreachable)\n");
}

int main(int argc, char const *argv[])
{
    // Set stack size
    if (exC_global_setup(50, 0))
    {
        fprintf(stderr, "Failed to initialize exCept\n");
        exit(EXIT_FAILURE);
    }
    if (exC_thrd_setup())
    {
        fprintf(stderr, "Failed to initialize exCept\n");
        exit(EXIT_FAILURE);
    }

    // Use it like this
    TRY
    {
        fprintf(stderr, "TRY\n");
        second_test_func();
    }
    CATCH(1)
    {
        fprintf(stderr, "CATCH(1)\n");
    }
    CATCH(2)
    {
        fprintf(stderr, "CATCH(2)\n");
    }
    CATCH(3)
    {
        fprintf(stderr, "CATCH(3)\n");
    }
    CATCH(e) // (or `CATCH()`, if you don't care about the exception value)
    {
        fprintf(stderr, "Unknown exception %d. Terminating process\n", e);
        exC_thrd_deinit();
        exC_global_deinit();
        TERMINATE(EXIT_FAILURE);
    }
    END_TRY;

    exC_thrd_deinit();
    exC_global_deinit();
    return 0;
}
```

Run it with :

```shell
$ gcc test.c exCept.c -o test
$ ./test
  TRY
  second_test_func
  first_test_func
  CATCH(3)
```

When using exCept in a multithreaded environment, it is advised that you exit a thread either by returning normally, `thrd_exit()`, `pthread_exit()`, or the appropriate function w.r.t. the implementation you chose (see also ["Choosing implementation"](#choosing-implementation)).
Else you can manually use `exC_thrd_deinit()` to clean thread-specific data for this thread - i.e. the `jmp_buf` stack, and the `WHAT` buffer, which are both stored in TSS - or just rely on your operating system if you do not use too much threads.

In a similar way, in case you want to stop the exception handling system at a particular point of execution, you can call `exC_global_deinit()` to delete the TSS keys.[^1]

[^1] At this point, no mechanism has been implemented to stop and restart the exception handling system.

## More details

### Nested `TRY / CATCH` blocks

In previous commits, macro `TRY` was accepting an argument to distinguish between different nested `TRY` blocks. The current version is now supporting nested `TRY` blocks without needing any arguments, if and only if your preprocessor has either `__COUNTER__` or `__LINE__` defined.

### Uncaught exceptions

Not handling an exception with exCept's `TRY / CATCH` blocks doesn't matter. You will just fall in the empty default case of the underlying switch, and then quit the `TRY / CATCH` block.

### Type and values of exceptions

There is no predefined exception. It's up to you to define your own exception codes. Default type of exceptions is `unsigned int`. To change this, compile with `-DEXCEPT_EXCEPTION_TYPE=size_t`, for example, or `#define EXCEPT_EXCEPTION_TYPE size_t` in `exCept_user_config.h`.

> **WARNING**
>
> There is one restriction on exception values : they **must** be between 1 and 512 (both included in the range). This 512 value (actually `CHAOS_PP_LIMIT_MAG`) is due to the limitations of chaos-pp, the metaprogramming library used to support multiple flavours of `CATCH` statements (see in sections below) : based on the argument provided to `CATCH`, it expands to different things.
>
> There is another restriction related to the previous facts : you can not define your exceptions has an `enum`. If you want to name your exceptions, please use macros. If you don't, then writing `CATCH(EXCEPTION_FOO)` (where EXCEPTION_FOO appears in an `enum`) will expand to the same thing as for instance `CATCH(e)`, instead of expanding to the same as `CATCH(2)`, for example, if `EXCEPTION_FOO == 2`. This is again because of `EXCEPTION_FOO` not directly expanding to a number between 1 and 512 (both included), and thus not treated as a number. If you really want to make sure your exception will be treated as it should be, verify that `CHAOS_PP_IS_NUMERIC(/* your exception name */)` expands to 1.

### Rethrowing an exception

exCept supports rethrowing an exception within a `CATCH` block :

```c
void func2(void)
{
    THROW(EXCEPTION_FOO);
}

void func1(void)
{
    TRY
    {
        func2();
    }
    CATCH(EXCEPTION_FOO)
    {
        THROW();
    }
    END_TRY;
}

int main(int argc, char*argv[])
{
    TRY
    {
        func1();
    }
    CATCH(EXCEPTION_FOO)
    {
        // handle EXCEPTION_FOO here
    }
    END_TRY;
}
```

You probably noticed the "call" to `THROW` without any argument, wich is reserved for this rethrowing purpose within `CATCH` blocks.

### Different flavours of `CATCH`

#### Catching a known exception

This is litteraly what we wrote until there :

```c
#define EXCEPTION_FOO 2
// but NOT enum {EXCEPTION_FOO = 2, ...};
TRY
{
    // blah blah blah
}
CATCH(1)
{
    // blah blah blah again
}
CATCH(EXCEPTION_FOO)
{
    // final blah blah blah
}
END_TRY;
```

#### Idiomatic `CATCH(e)`

```c
TRY
{
    // ...
}
CATCH(1)
{
    // ...
}
CATCH(e)
{
    if (e == EXCEPTION_FOO)
    {
        // ...
    }
    // ...
}
END_TRY;
```

#### Simply `CATCH()`

```c
TRY
{
    // ...
}
CATCH(1)
{
    // ...
}
CATCH()
{
    // ...
}
END_TRY;
```

Note that doing this way, you can not know which exception has been thrown.

### On the use of non-volatile variables

Since exCept uses setjmp and longjmp internally (probably as any other exception library in C), any variable in the scope of setjmp is not guaranted to preserve all the changes made to it in a TRY block when an exception is caught. exCept provides utility macros to handle the potential needs to preserve changes :

```c
int i = 2;
int j = 4;
SYNC_CHANGES(i, j);
TRY
{
    // swap i and j
    i += j;
    j = i - j;
    i = i - j;
    SAVE(i, j);
    THROW(1);
}
CATCH()
{
    // i and j are not swapped
    LOAD(i, j);
    // i and j are swapped
}
END_TRY;
```

Or, alternatively :

```c

int i = 2;
int j = 4;
SYNC_CHANGES(i, j);
TRY
{
    // swap i and j
    // not always optimal to manipulate only volatile variables,
    // since their use can not be fully optimized by compilers
    VAR(i) += VAR(j);
    VAR(j) = VAR(i) - VAR(j);
    VAR(i) = VAR(i) - VAR(j);
    THROW(1);
}
CATCH()
{
    // VAR(i) and VAR(j) are swapped but i and j are not
    LOAD(i, j);
    // i and j are swapped
}
END_TRY;
```

`SYNC_CHANGES(i)` declares a variable with the type of i, but `volatile`. It **must** be used (after any following `TRY` / `CATCH`) if you want to use the other utility macros. This volatile equivalent can be directly used via `VAR(i)` once it has been declared. If you do not want your code to be suboptimized, you can still use these volatile variables only at the end of the use of `i` and `j` to store `i` and `j` in their volatile equivalent with `SAVE(i, j)`. Then, if you need to restore `i` and `j` after an exception has been caught, then just `LOAD(i, j)`.

> **WARNING**
>
> If you need to manipulate the direct value of pointers (i.e. the memory address they store, not what they point to), then you'll have to declare your volatile pointer yourself, since (at least for now) `SYNC_CHANGES` can declare for example the `volatile FILE* f` equivalent of `FILE* f`, but not `FILE* volatile f`.

### On the internal use of `typeof`

If `typeof` isn't available with you compiler, but your compiler has a similar keyword, then `#define EXCEPT_TYPEOF /* your typeof */` will do the job.

### Choosing implementation

As mentioned before, exCept uses thread-specific storage to store `jmp_buf` stack and `WHAT` buffer. Hence, it is possible to choose between the three currently available threading implementations by defining a bunch of macros. For instance, in the file `exCept_user_config.h`, you could :

- `#define EXCEPT_USE_THREADS_H` to use `<threads.h>`
- `#define EXCEPT_USE_PTHREADS` to use `<pthread.h>`
- `#define EXCEPT_USE_WINDOWS_THREADS` to use `<windows.h>`[^2]

The fallback in case nothing is defined is to use C11 standard `<threads.h>`'s TSS implementation.

In the future, it may be possible to :

- Provide a flag to select a mono-threaded implementation
- Use a custom blocking hastable implementation to not depend on any TSS implementation, and let the user specify a mutex type (see the comment at the top of [exCept.c](./exCept.c#L34))

[^2] Windows implementation is not yet perfect and probably needs to be tested

## Documentation

For a more complete documentation, you can also look the source code and the doxygen-ready comments.

### Index of available macros

```c

/*
 * Same semantics as C++'s `try` for instance. Use it as:
 *   TRY { ... }
 */
#define TRY

/*
 * Same semantics as C++'s `catch` for instance. Can be "called" in three different ways:
 *   - CATCH(<number>) { ... }
 *       <number> being either an actual integer literal with no suffix or
 *       a macro expanding to an actual integer literal with no suffix
 *   - CATCH(<identifier>) { ... }
 *       <identifier> being a valid C identifier created within this `CATCH` scope,
 *       that you can use however you want. Note that this use is not compatible
 *       with the following one
 *   - CATCH() { ... }
 *       a sort of "default" case, where you can not use the exception value, but still
 *       can use the string provided via `WHAT` macro. This use is not compatible with the
 *       previous one
 */
#define CATCH(...)

/*
 * Same semantics as C++'s `throw` for instance. Can be "called" in three different ways:
 *   - THROW(<number>);
 *       <number> being either an actual integer literal with no suffix or
 *       a macro expanding to an actual integer literal with no suffix
 *   - THROW(<number>, <string>);
 *       <number> being the same as the previous case, and <string> being
 *       a valid C string literal
 *   - THROW();
 *       which makes the `CATCH` block rethrow the previous exception, with
 *       the same `WHAT` message
 */
#define THROW(...)

/*
 * Use it to delimit the end of a `TRY-CATCH` block
 */
#define END_TRY

/*
 * Declare an associated volatile variable for each parameter passed to it. Usefull to preserve
 * changes that could otherwise be discarded by `longjmp`
 * There is no limitation on the number of argument you can pass to this macro
 */
#define SYNC_CHANGES(...)

/*
 * For each parameter passed to it, saves its value in its associated volatile variable
 * (previously declared with `SYNC_CHANGES`)
 * There is no limitation on the number of argument you can pass to this macro
 */
#define SAVE(...)

/*
 * For each parameter passed to it, copy the content of its associated volatile variable to it
 * There is no limitation on the number of argument you can pass to this macro
 */
#define LOAD(...)

/*
 * Expands to the associated volatile variable of its parameter
 */
#define VAR(variable)

/*
 * Tries to imitate C++'s `terminate` behaviour
 */
#define TERMINATE(status, ...)

/*
 * Returns the last exception message, as in C++
 */
#define WHAT

/*
 * Tries to imitate the behaviour of C++'s noexcept. Use it like:
 *   int func(void)
 *   {
 *        NOEXCEPT
 *        {
 *             ...
 *             return 0;
 *        }
 *        END_NOEXCEPT;
 *   }
 */
#define NOEXCEPT

/*
 * Tries to imitate the behaviour of C++'s noexcept. See above
 */
#define END_NOEXCEPT

```

### Index of available functions

```c
/*
 * Setup the global state of the exception handling system
 */
int  exC_global_setup(size_t stack_size, exC_flags_t flags);

/*
 * Setup the thread state of the exception handling system
 */
int  exC_thrd_setup(void);

/*
 * Deinit the thread local resources used by the exception handling system
 */
void exC_thrd_deinit(void);

/*
 * Deinit the global resources used by the exception handling system
 */
void exC_global_deinit(void);

/*
 * Define the termination handler, as used by the `TERMINATE` macro
 */
int  exC_set_term_handler(term_handler_t handler);
```
