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
    // SET STACK SIZE
    fprintf(stderr, "\n");
    fprintf(stderr, "SET STACK SIZE\n");
    if (exC_set_stack_size(10))
    {
        fprintf(stderr, "Stack size already set.\n");
    }
    if (exC_set_stack_size(20))
    {
        fprintf(stderr, "Stack size already set.\n");
    }
    fprintf(stderr, "\n");

    // FIRST TEST
    fprintf(stderr, "\n");
    fprintf(stderr, "FIRST TEST\n");
    TRY 
    {
        fprintf(stderr, "TRY\n");
        THROW(1);
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
    fprintf(stderr, "\n");

    // SECOND TEST
    fprintf(stderr, "\n");
    fprintf(stderr, "SECOND TEST\n");
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
    fprintf(stderr, "\n");
    return 0;
}
