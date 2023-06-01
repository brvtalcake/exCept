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

void third_test_func(void)
{
    fprintf(stderr, "third_test_func\n");
    TRY(1)
    {
        fprintf(stderr, "TRY (third_test_func)\n");
        first_test_func();
        fprintf(stderr, "TRY (third_test_func) (unreachable)\n");
    }
    CATCH(1)
    {
        fprintf(stderr, "CATCH(1) (third_test_func)\n");
        THROW();
        fprintf(stderr, "CATCH(1) (third_test_func) (unreachable)\n");
    }
    CATCH(2)
    {
        fprintf(stderr, "CATCH(2) (third_test_func)\n");
        THROW();
        fprintf(stderr, "CATCH(2) (third_test_func) (unreachable)\n");
    }
    CATCH(3)
    {
        fprintf(stderr, "CATCH(3) (third_test_func)\n");
        THROW();
        fprintf(stderr, "CATCH(3) (third_test_func) (unreachable)\n");
    }
    END_TRY;
    fprintf(stderr, "third_test_func (unreachable)\n");
}

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;

    // SET STACK SIZE
    fprintf(stderr, "\n");
    fprintf(stderr, "SET STACK SIZE\n");
    if (exC_set_stack_size(3))
    {
        fprintf(stderr, "Stack size already set.\n");
    }
    if (exC_set_stack_size(20))
    {
        fprintf(stderr, "Stack size already set.\n");
    }
    fprintf(stderr, "\n");

    // FIRST TEST
    // Basic functionnalities
    fprintf(stderr, "\n");
    fprintf(stderr, "FIRST TEST\n(Basic functionnalities)\n");
    TRY(1)
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
    // Throwing from a function
    fprintf(stderr, "\n");
    fprintf(stderr, "SECOND TEST\n(Throwing from a function)\n");
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
    fprintf(stderr, "\n");

    // THIRD TEST
    // Rethrowing to an outer TRY block that doesn't exist
    fprintf(stderr, "\n");
    fprintf(stderr, "THIRD TEST\n(Rethrowing to an outer TRY block that doesn't exist)\n");
    TRY(1)
    {
        fprintf(stderr, "TRY\n");
        THROW(2);
    }
    CATCH(1)
    {
        fprintf(stderr, "CATCH(1)\n");
    }
    CATCH(2)
    {
        fprintf(stderr, "CATCH(2)\n");
        THROW(); // won't do anything, because it is not nested in a previous TRY block
    }
    CATCH(3)
    {
        fprintf(stderr, "CATCH(3)\n");
    }
    END_TRY;
    fprintf(stderr, "\n");

    // FOURTH TEST
    // Rethrowing to an outer TRY block, nested TRY blocks in the same scope
    fprintf(stderr, "\n");
    fprintf(stderr, "FOURTH TEST\n(Rethrowing to an outer TRY block, nested TRY blocks in the same scope)\n");
    TRY(1)
    {
        fprintf(stderr, "TRY (outer)\n");
        TRY(2)
        {
            fprintf(stderr, "TRY (inner)\n");
            THROW(2); // will be caught by the outer TRY block
        }
        CATCH(1)
        {
            fprintf(stderr, "CATCH(1) (inner)\n");
        }
        CATCH(2)
        {
            fprintf(stderr, "CATCH(2) (inner)\n");
            THROW();
            fprintf(stderr, "CATCH(2) (unreachable)\n");
        }
        CATCH(3)
        {
            fprintf(stderr, "CATCH(3) (inner)\n");
        }
        END_TRY;
    }
    CATCH(1)
    {
        fprintf(stderr, "CATCH(1) (outer)\n");
    }
    CATCH(2)
    {
        fprintf(stderr, "CATCH(2) (outer)\n");
    }
    CATCH(3)
    {
        fprintf(stderr, "CATCH(3) (outer)\n");
    }
    END_TRY;
    fprintf(stderr, "\n");

    // FIFTH TEST
    // Rethrowing from a function to another
    fprintf(stderr, "\n");
    fprintf(stderr, "FIFTH TEST\n(Rethrowing from a function to another)\n");
    TRY(1)
    {
        fprintf(stderr, "TRY\n");
        third_test_func();
        THROW(1); // won't throw
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

    // SIXTH TEST
    // "Synchronizing" changes (use volatile variables)
    // See https://godbolt.org/z/8r7rP6vh8
    fprintf(stderr, "\n");
    fprintf(stderr, "SIXTH TEST\n(\"Synchronizing\" changes)\n");
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;
    fprintf(stderr, "i (before TRY) = %u\n", i);
    fprintf(stderr, "j (before TRY) = %u\n", j);
    fprintf(stderr, "k (before TRY) = %u\n", k);
    SYNC_CHANGES(i, j);
    TRY(1)
    {
        k = 1;
        
        i = 1;
        SAVE(i);
        
        VAR(j) = 1;
        
        fprintf(stderr, "Modifying i, j and k (in TRY)\n");
        
        THROW(1);
    }
    CATCH(1)
    {
        LOAD(i, j);
        fprintf(stderr, "CATCH(1)\n");
        fprintf(stderr, "i (in CATCH) = %u\n", i);
        fprintf(stderr, "j (in CATCH) = %u\n", j);
        fprintf(stderr, "k (in CATCH) = %u\n", k);
    }
    END_TRY;
    LOAD(i, j);
    fprintf(stderr, "i (after TRY) = %u\n", i);
    fprintf(stderr, "j (after TRY) = %u\n", j);
    fprintf(stderr, "k (after TRY) = %u\t(Could actually be anything. For more informations, see https://godbolt.org/z/8r7rP6vh8 (example taken from cppreference and modified))\n", k);
    fprintf(stderr, "\n");

    // SEVENTH TEST
    // Different flavours of CATCH
    fprintf(stderr, "\n");
    fprintf(stderr, "SEVENTH TEST\n(Different flavours of CATCH)\n");
    TRY(1)
    {
        fprintf(stderr, "TRY\n");
        THROW(4);
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
    CATCH(e)
    {
        fprintf(stderr, "CATCH(e)\n");
        fprintf(stderr, "e = %d\n", e);
    }
    END_TRY;
    fprintf(stderr, "\n");

    // EIGHTH TEST
    // Different flavours of CATCH
    fprintf(stderr, "\n");
    fprintf(stderr, "EIGHTH TEST\n(Different flavours of CATCH)\n");
    TRY(1)
    {
        fprintf(stderr, "TRY\n");
        THROW(4);
    }
    CATCH()
    {
        fprintf(stderr, "CATCH() something\n");
    }
    END_TRY;
    fprintf(stderr, "\n");

    // NINTH TEST
    // No CATCH
    fprintf(stderr, "\n");
    fprintf(stderr, "NINTH TEST\n(No CATCH)\n");
    TRY(1)
    {
        fprintf(stderr, "TRY\n");
        THROW(4);
    }
    END_TRY;
    fprintf(stderr, "\n");

    // TENTH TEST
    // ...
    fprintf(stderr, "\n");
    fprintf(stderr, "TENTH TEST\n(...)\n");
    #define EXCEPTION_FOO 1
    #define EXCEPTION_BAR 2
    #define EXCEPTION_BAZ 3
    TRY(1)
    {
        fprintf(stderr, "TRY\n");
        THROW(EXCEPTION_FOO);
    }
    CATCH(EXCEPTION_FOO)
    {
        fprintf(stderr, "CATCH(EXCEPTION_FOO) where EXCEPTION_FOO = %d\n", EXCEPTION_FOO);
    }
    CATCH(EXCEPTION_BAR)
    {
        fprintf(stderr, "CATCH(EXCEPTION_BAR) where EXCEPTION_BAR = %d\n", EXCEPTION_BAR);
    }
    CATCH(EXCEPTION_BAZ)
    {
        fprintf(stderr, "CATCH(EXCEPTION_BAZ) where EXCEPTION_BAZ = %d\n", EXCEPTION_BAZ);
    }
    END_TRY;
    fprintf(stderr, "\n");

    return 0;
}
