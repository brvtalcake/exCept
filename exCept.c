#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "exCept.h"

static thread_local jmp_buf **stack = NULL;
static thread_local bool stack_created = false;

static thread_local size_t stack_size = 0;
static thread_local bool stack_size_set = false;

static thread_local size_t stack_top = 0;

EXCEPT_API int exC_set_stack_size(size_t size)
{
    if (stack_created)
    {
        return -1;
    }
    stack_size = size;
    stack_size_set = true;
    return 0;
}

EXCEPT_API int exC_is_stack_size_set(void)
{
    return stack_size_set ? 1 : 0;
}

EXCEPT_API int exC_is_stack_created(void)
{
    return stack_created ? 1 : 0;
}

EXCEPT_API void exC_create_stack(void) 
{
    if (stack_created || !stack_size_set)
    {
        return;
    }
    stack = malloc(stack_size * sizeof(jmp_buf*));
    stack_created = true;
    stack_top = 0;
    if (atexit(exC_destroy_stack)) 
    {
        exit(EXIT_FAILURE);
    }
}

EXCEPT_API void exC_destroy_stack(void)
{
    if (!stack_created) 
    {
        return;
    }
    free(stack);
    stack_created = false;
    stack_top = 0;
}

EXCEPT_API int exC_push_stack(jmp_buf* env)
{
    if (!stack_created) 
    {
        return -1;
    }
    if (stack_top >= stack_size) 
    {
        fprintf(stderr, "Exception stack overflow.\n");
        exit(EXIT_FAILURE);
    }
    stack[stack_top++] = env;
    return 0;
}

EXCEPT_API void exC_pop_stack(void)
{
    if (!stack_created || stack_top == 0) 
    {
        return;
    }
    stack[--stack_top] = NULL;
}

EXCEPT_API void exC_rewind(int except)
{
    if (!stack_created || stack_top == 0) 
    {
        return;
    }
    longjmp(*stack[--stack_top], except);
}
