# exCept

exCept is a two files C "library" providing exception handling in pure C.

> **Note**
>
> `FINALLY` is not yet supported, and probably won't ever be supported, because it is not really possible (or at least very hard) to write a "real" `finally` in C. This problem is similar to a `defer`, wich is usually imitated in C using a `goto label` or something like that to deallocate the ressources in just one place in your function, etc... The same is possible with exCept.
> However, if you want a "real" (but non-portable) defer macro, check [this repo](https://github.com/brvtalcake/defer).

## TODO

- [x] Write a Makefile
- [ ] Make it possible to just `CATCH(e)` and handle the exception differently according to its value
- [ ] Potentially write a `FINALLY` macro
- [ ] Write a MAY_THROW macro for function (wrapper around _Noreturn and / or compiler specific function attributes)
- [ ] Document the usage of `SYNC_CHANGES`, `VAR`, `LOAD`, and `SAVE`

## Usage

First thing you need to do is to set the size of the jmp_buf stack (a kind of exception stack, in our case, or even the amount of `TRY / CATCH` statements you can nest), with :

```c
exC_set_stack_size(42);
```

Then you can start using the provided macros as you would use them in higher level programming languages. Here is a basic example of use :

```c
// If you want to use lowercase keywords (at your own risk), then uncomment the following line
// (it must appear before including header, and you can not use both syntaxes in the same time)

/* #define EXCEPT_LOWERCASE */

#include <stdio.h>
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
    if (exC_set_stack_size(10))
    {
        fprintf(stderr, "Stack size already set.\n");
    }

    // Use it like this
    TRY(1)
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
    END_TRY;

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

The allocated `jmp_buf` array is supposed to be deallocated automatically due to a call to `atexit`, but if you want to make sure it is deallocated, then call ...

```c
exC_destroy_stack();
```

... at the end of your program.

## More details

### Nested `TRY / CATCH` blocks

The reason why TRY accepts one argument is to make you able to write nested `TRY / CATCH` in the same scope. For instance, to write two `TRY / CATCH` blocks in the same function :

```c
void func(void)
{
    // the `1` as argument in the `TRY` below means it's the first level of `TRY / CATCH` in this scope
    TRY(1) 
    {
        // ...
        TRY(2)
        {
            // ...
        }
        CATCH(EXCEPTION_BAR)
        {
            // ...
        }
        END_TRY;
    }
    CATCH(EXCEPTION_FOO)
    {
        // ...
    }
    END_TRY;
}
```

Alternatively, you could even write :

```c
void func(void)
{
    // the `1` as argument in the `TRY` below means it's the first level of `TRY / CATCH` in this scope
    TRY(15987456) 
    {
        // ...
        TRY(abcd)
        {
            // ...
        }
        CATCH(EXCEPTION_BAR)
        {
            // ...
        }
        END_TRY;
    }
    CATCH(EXCEPTION_FOO)
    {
        // ...
    }
    END_TRY;
}
```

> **Note**
>
> The only important thing is to have different numbers (or even letters if you want, actually) in each level of `TRY / CATCH`. The reason for this is that `TRY` declares a local variable by concatenating the `exC_`, `env`, and the argument passed in the `TRY` macro. If you put the same thing in two nested TRY, you will have a compilation error.

## Uncaught exceptions

Not handling an exception with exCept's `TRY / CATCH` blocks doesn't matter. You will just fall in the empty default case of the underlying switch, and then quit the `TRY / CATCH` block.

## Type and values of exceptions

There is no predefined exception. It's up to you to define your own exception codes. Default type of exceptions is `unsigned int`. To change this, compile with `-D EXCEPT_EXCEPTION_TYPE=size_t`.

> **WARNING**
>
> There is one restriction on exception values : they **must** be between 1 and 512 (both included in the range). This 512 value (actually `CHAOS_PP_LIMIT_MAG`) is due to the limitations of chaos-pp, the metaprogramming library used to support multiple flavours of `CATCH` statements (see in sections below) : based on the argument provided to `CATCH`, it expands to different things.
>
> There is another restriction related to the previous facts : you can not define your exceptions has an `enum`. If you want to name your exceptions, please use macros. If you don't, then writing `CATCH(EXCEPTION_FOO)` (where EXCEPTION_FOO appears in an `enum`) will expand to the same thing as for instance `CATCH(e)`, instead of expanding to the same as `CATCH(2)`, for example, if `EXCEPTION_FOO == 2`. This is again because of `EXCEPTION_FOO` not directly expanding to a number between 1 and 512 (both included), and thus not treated as a number. If you really want to make sure your exception will be treated as it should be, verify that `CHAOS_PP_IS_NUMERIC(/* your exception name */)` expands to 1.

## Rethrowing an exception

exCept supports rethrowing an exception within a `CATCH` block :

```c
void func2(void)
{
    THROW(EXCEPTION_FOO);
}

void func1(void)
{
    TRY(1)
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
    TRY(1)
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

## Different flavours of `CATCH`

### Catching a known exception

This is litteraly what we wrote until there :

```c
#define EXCEPTION_FOO 2
// but NOT enum {EXCEPTION_FOO = 2, ...};
TRY(1)
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

### Idiomatic `CATCH(e)`

```c
TRY(1)
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

### Simply `CATCH()`

```c
TRY(1)
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

## On the use of non-volatile variables

Since exCept uses setjmp and longjmp internally (probably as any other exception library in C), any variable in the scope of setjmp is not guaranted to preserve all the changes made to it in a TRY block when an exception is caught. exCept provides utility macros to handle the potential needs to preserve changes :

```c
{
    int i = 2;
    int j = 4;
    SYNC_CHANGES(i, j);
    TRY(1)
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
}
```

Or, alternatively :

```c
{
    int i = 2;
    int j = 4;
    SYNC_CHANGES(i, j);
    TRY(1)
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
}
```

`SYNC_CHANGES(i)` declares a variable with the type of i, but `volatile`. It **must** be used (after any following `TRY` / `CATCH`) if you want to use the other utility macros. This volatile equivalent can be directly used via `VAR(i)` once it has been declared. If you do not want your code to be suboptimized, you can still use these volatile variables only at the end of the use of `i` and `j` to store `i` and `j` in their volatile equivalent with `SAVE(i, j)`. Then, if you need to restore `i` and `j` after an exception has been caught, then just `LOAD(i, j)`.

> **WARNING**
>
> If you need to manipulate the direct value of pointers (i.e. the memory address they store, not what they point to), then you'll have to declare your volatile pointer yourself, since (at least for now) `SYNC_CHANGES` can declare for example the `volatile FILE* f` equivalent of `FILE* f`, but not `FILE* volatile f`.

## On the internal use of `typeof`

If `typeof` isn't available with you compiler, but your compiler has a similar keyword, then `#define EXCEPT_TYPEOF /* your typeof */` will do the job.
