/*
 * MIT License
 * 
 * Copyright (c) 2023 Axel PASCON
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "exCept.h"

static thread_local jmp_buf **stack = NULL;
static thread_local bool stack_created = false;

static thread_local size_t stack_top = 0;
static thread_local EXCEPT_EXCEPTION_TYPE last_exception;

static size_t stack_size = 0;
static bool stack_size_set = false;


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

EXCEPT_API void exC_unwind(EXCEPT_EXCEPTION_TYPE except)
{
    if (!stack_created || stack_top == 0) 
    {
        return;
    }
    last_exception = except;
    longjmp(*stack[--stack_top], (int) except); // `except` must be 0 < except <= 512
}

EXCEPT_API EXCEPT_EXCEPTION_TYPE exC_last_exception(void)
{
    return last_exception;
}

// TODO
#if 0
EXCEPT_API void exC_terminate(void)
{
    if (!stack_created) 
    {
        return;
    }
    
}
#endif