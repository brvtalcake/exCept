# exCept

exCept is a two files C "library" providing exception handling in pure C. `FINALLY` is not yet supported, but will be supported quite soon I guess.

## TODO

- Write a Makefile
- Create a `FINALLY` macro

## Usage

First thing you need to do is to set the size of the jmp_buf stack (a kind of exception stack, in our case, or even the amount of TRY / CATCH statements you can nest), with :

```c
exC_set_stack_size(42);
```

Then you can start using the provided macros as you would use them in higher level programming languages :

```c
// If you want to use lowercase keywords (at your own risk), then uncomment the following line
// (it must appear before including header, and you can not use both syntaxes)

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

The allocated array is supposed to be deallocated automatically due to a call to `atexit`, but if you want to make sure it is deallocated, then call ...

```c
exC_destroy_stack();
```

... at the end of your program.
