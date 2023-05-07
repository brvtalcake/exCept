# exCept

exCept is a two files C "library" providing exception handling in pure C.

> **Note**
> `FINALLY` is not yet supported, and probably won't ever be supported, because it is not really possible to build a "real" `finally` in C. This problem is similar to a `defer`, wich is usually imitated in C using a `goto label` or something like that to deallocate the ressources in just one place in your function, etc... The same is possible with exCept.

## TODO

- [ ] Write a Makefile
- [ ] Make it possible to just `CATCH(e)` and handle the exception differently according to its value

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
> The only important thing is to have different numbers (or even letters if you want, actually) in each level of `TRY / CATCH`. The reason for this is that `TRY` declares a local variable by concatenating the `exC_`, `env`, and the argument passed in the `TRY` macro. If you put the same thing in two nested TRY, you will have a compilation error.

## Uncaught exceptions

Not handling an exception with exCept's `TRY / CATCH` blocks doesn't matter. You will just fall in the empty default case of the underlying switch, and then quit the `TRY / CATCH` block.

## Type and values of exceptions

There is no predefined exception. It's up to you to define your own exception codes. Default type of exceptions is `unsigned int`. To change this, compile with `-D EXCEPT_EXCEPTION_TYPE=size_t`

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
