# exCept

exCept is a two files C "library" providing exception handling in pure C.

## TODO

- Write a Makefile
- Create a `FINALLY` macro

## Usage

First thing you need to do is to set the size of the jmp_buf stack (a kind of exception stack, in our case), with :

```c
exC_set_stack_size(42);
```

Then you can start using the provided macros as you would use them in higher level programming languages :

```c
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
