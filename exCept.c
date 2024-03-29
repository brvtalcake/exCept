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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdnoreturn.h>

#include "exCept_user_config.h"

// TODO: Use something else than tss, let the user provide it's own mutex type and its own way to distinguish threads, 
//       and use a custom key-value map implementation to store the exception stack / what buffer for each thread. For
//       instance, on Raspberry pi pico, user could use the SDK's `mutex_t` type, and use the `get_core_num()` function.

// TODO: Provide a `EXCEPT_ONE_THREAD` flag

#if EXCEPT_TERM_HANDLER_ARGC != 1
    #include <stdarg.h>
#endif

#if !defined(EXCEPT_WHAT_MAX_SIZE)
    #define EXCEPT_WHAT_MAX_SIZE (256 * 2 * 2 * 2)
#endif

#undef THRD_SUCCESS
#undef TSS_T
#undef TSS_CREATE
#undef TSS_GET
#undef TSS_SET
#undef TSS_DELETE
#undef ONCE_FLAG
#undef ONCE_INIT
#undef CALL_ONCE

// TODO: Add support for other threading libraries
#if defined(EXCEPT_USE_THREADS_H) || (!defined(EXCEPT_USE_PTHREADS) && !defined(EXCEPT_USE_WINDOWS_THREADS))
    #include <threads.h>
    // A common return value when success
    #define THRD_SUCCESS thrd_success
    // The type of the thread-specific storage key
    #define TSS_T tss_t
    // Create a thread-specific storage key. Should be called only once per key.
    #define TSS_CREATE(key, destructor) tss_create(key, destructor)
    // Get the value associated with the key, in the current thread
    #define TSS_GET(key) tss_get(key)
    // Set the value associated with the key, in the current thread
    #define TSS_SET(key, value) tss_set(key, value)
    // Delete the key. Should be called only once per key.
    #define TSS_DELETE(key) tss_delete(key)
    #define ONCE_FLAG once_flag
    #define ONCE_INIT ONCE_FLAG_INIT
    #define CALL_ONCE(flag, func) call_once(flag, func)
#elif defined(EXCEPT_USE_PTHREADS)
    #include <pthread.h>
    #define THRD_SUCCESS 0
    #define TSS_T pthread_key_t
    #define TSS_CREATE(key, destructor) pthread_key_create(key, destructor)
    #define TSS_GET(key) pthread_getspecific(key)
    #define TSS_SET(key, value) pthread_setspecific(key, value)
    #define TSS_DELETE(key) pthread_key_delete(key)
    #define ONCE_FLAG pthread_once_t
    #define ONCE_INIT PTHREAD_ONCE_INIT
    #define CALL_ONCE(flag, func) pthread_once(flag, func)
#elif defined(EXCEPT_USE_WINDOWS_THREADS)
    // TODO: Test/Improve Windows implementation
    #include <windows.h>
    #define THRD_SUCCESS 0
    #define TSS_T DWORD
    #define TSS_CREATE(key, destructor) ((*(key) = TlsAlloc()) == TLS_OUT_OF_INDEXES ? 1 : THRD_SUCCESS)
    #define TSS_GET(key) TlsGetValue(*(key))
    #define TSS_SET(key, value) TlsSetValue(*(key), (value))
    #define TSS_DELETE(key) TlsFree(*(key))
    #define ONCE_FLAG INIT_ONCE
    #define ONCE_INIT INIT_ONCE_STATIC_INIT
    #define CALL_ONCE(flag, func) InitOnceExecuteOnce(flag, func, NULL, NULL)
#else
    // It won't happen because of the fallback when nothing specified
#endif

#if defined(IF_FLAG)
    #undef IF_FLAG
#endif
#define IF_FLAG(var, flag) if ((var) & (flag))
#if defined(IF_NO_FLAG)
    #undef IF_NO_FLAG
#endif
#define IF_NO_FLAG(var, flag) if (!((var) & (flag)))
#define EXCEPT_SOURCE 1

#include "exCept.h"

static bool global_setup_done = false;
static thread_local bool thread_setup_done = false;

// "Real" type: jmp_buf**
static TSS_T stack;
static ONCE_FLAG stack_once = ONCE_INIT;
static thread_local bool stack_created = false;

static thread_local volatile size_t stack_top = 0;
static thread_local volatile EXCEPT_EXCEPTION_TYPE last_exception;

static size_t stack_size = 0;
static bool stack_size_set = false;

static term_handler_t term_handler = &exit;

static exC_flags_t user_flags = 0;

/*
 * "Real" type: volatile char* volatile
 * Users will be able to optionally provide a string to THROW, something like THROW(<unsigned int error code>, <potential string>).
 * We thus need to store it somehow, so the user could then use a WHAT macro to retrieve it.
 */
static TSS_T last_exception_what;
static ONCE_FLAG last_exception_what_once = ONCE_INIT;

static inline void exC_set_stack_size(size_t size);
static inline int exC_create_stack(void);

static void stack_tss_create(void);
static void last_exception_what_tss_create(void);

EXCEPT_API
int exC_is_global_setup_done(void)
{
    return global_setup_done && stack_size_set ? 1 : 0;
}

EXCEPT_API
int exC_is_thread_setup_done(void)
{
    return thread_setup_done && stack_created ? 1 : 0;
}

EXCEPT_API
int exC_global_setup(size_t stack_size, exC_flags_t flags)
{
    if (global_setup_done)
        return 0;
    user_flags = flags;
    exC_set_stack_size(stack_size);
    global_setup_done = true;
    return 0;
}

EXCEPT_API
int exC_thrd_setup(void)
{
    if (!global_setup_done)
        return -1;
    if (thread_setup_done)
        return 0;
    CALL_ONCE(&stack_once, stack_tss_create);
    CALL_ONCE(&last_exception_what_once, last_exception_what_tss_create);
    if (exC_create_stack() != 0)
        return -1;
    thread_setup_done = true;
    return 0;
}

static inline void exC_set_stack_size(size_t size)
{
    if (stack_size_set)
        return;
    stack_size = size;
    stack_size_set = true;
}

EXCEPT_API
int exC_is_stack_size_set(void)
{
    return stack_size_set ? 1 : 0;
}

EXCEPT_API
int exC_is_stack_created(void)
{
    return stack_created ? 1 : 0;
}

static inline int exC_create_stack(void)
{
    if (stack_created)
        return 0;
    if (!stack_size_set)
        return -1;
    if (TSS_SET(stack, calloc(stack_size, sizeof(jmp_buf*))) != THRD_SUCCESS)
        return -1;
    if (TSS_SET(last_exception_what, calloc(EXCEPT_WHAT_MAX_SIZE, sizeof(char))) != THRD_SUCCESS)
    {
        TSS_DELETE(stack);
        return -1;
    }
    if (TSS_GET(stack) == NULL || TSS_GET(last_exception_what) == NULL)
    {
        if (TSS_GET(stack) != NULL)
            TSS_DELETE(stack);
        if (TSS_GET(last_exception_what) != NULL)
            TSS_DELETE(last_exception_what);
        return -1;
    }
    stack_created = true;
    stack_top = 0;
    return 0;
}

EXCEPT_API
int exC_push_stack(jmp_buf* env)
{
    if (!stack_created)
        return -1;
    if (stack_top >= stack_size) 
    {
        fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR:" P_RESET " Exception stack overflow.\n");
        exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);
    }
    jmp_buf** stack_ptr = TSS_GET(stack);
    if (stack_ptr == NULL)
        return -1;
    stack_ptr[stack_top++] = env;
    return 0;
}

EXCEPT_API
void exC_pop_stack(void)
{
    if (!stack_created || stack_top == 0)
        return;
    jmp_buf** stack_ptr = TSS_GET(stack);
    if (stack_ptr == NULL)
    {
        fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR:" P_RESET " Failed to pop exception stack.\n");
        exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);
    }
    stack_ptr[--stack_top] = NULL;
}

EXCEPT_API EXCEPT_NORETURN EXCEPT_SENTINEL_NULL(0)
void exC_unwind(EXCEPT_EXCEPTION_TYPE except, ...)
{
    if (!stack_created || stack_top == 0)
    {
        fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR:" P_RESET " Exception has been thrown without initializing the exception stack.\n");
        exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);
    }
    last_exception = except;
    va_list args;
    va_start(args, except);
    char* what = va_arg(args, char*);
    va_end(args);
    char* last_exception_what_ptr = TSS_GET(last_exception_what);
    if (what != NULL)
    {
        strncpy(last_exception_what_ptr, what, EXCEPT_WHAT_MAX_SIZE - 1);
        last_exception_what_ptr[EXCEPT_WHAT_MAX_SIZE - 1] = '\0';
    }
    else
        last_exception_what_ptr[0] = '\0';
    jmp_buf** stack_ptr = TSS_GET(stack);
    if (stack_ptr == NULL)
    {
        fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR:" P_RESET " Failed to unwind exception stack.\n");
        exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);
    }
    longjmp(*stack_ptr[--stack_top], (int) except); // `except` must satisfy 0 < except <= 512
}

EXCEPT_API
char* exC_last_exception_what(void)
{
    return TSS_GET(last_exception_what);
}

EXCEPT_API
EXCEPT_EXCEPTION_TYPE exC_last_exception(void)
{
    return last_exception;
}

EXCEPT_API
int exC_set_term_handler(term_handler_t handler)
{
    if (handler == NULL)
        return -1;
    term_handler = handler;
    return 0;
}

EXCEPT_API EXCEPT_NORETURN
void exC_terminate(int status, ...)
{
    if (term_handler == NULL)
        exit(status);

    // Because `exC_terminate` deletes those TSS keys, user should exit each thread before calling `exC_terminate`,
    // so the destructors are called and the memory is freed
    // (though it's not a big deal if they don't, since operating systems will clean up the memory anyway)    
    TSS_DELETE(stack);
    TSS_DELETE(last_exception_what);

#if (EXCEPT_TERM_HANDLER_ARGC == 2)
        va_list args;
        va_start(args, status);
        EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST) arg = va_arg(args, EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST));
        va_end(args);
        term_handler(status, arg);
#elif (EXCEPT_TERM_HANDLER_ARGC == 3)
        va_list args;
        va_start(args, status);
        EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST) arg1 = va_arg(args, EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST) arg2 = va_arg(args, EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST));
        va_end(args);
        term_handler(status, arg1, arg2);
#elif (EXCEPT_TERM_HANDLER_ARGC == 4)
        va_list args;
        va_start(args, status);
        EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST) arg1 = va_arg(args, EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST) arg2 = va_arg(args, EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST) arg3 = va_arg(args, EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST));
        va_end(args);
        term_handler(status, arg1, arg2, arg3);
#elif (EXCEPT_TERM_HANDLER_ARGC == 5)
        va_list args;
        va_start(args, status);
        EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST) arg1 = va_arg(args, EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST) arg2 = va_arg(args, EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST) arg3 = va_arg(args, EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST) arg4 = va_arg(args, EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST));
        va_end(args);
        term_handler(status, arg1, arg2, arg3, arg4);
#elif (EXCEPT_TERM_HANDLER_ARGC == 6)
        va_list args;
        va_start(args, status);
        EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST) arg1 = va_arg(args, EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST) arg2 = va_arg(args, EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST) arg3 = va_arg(args, EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST) arg4 = va_arg(args, EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(5, EXCEPT_TERM_HANDLER_TYPE_LIST) arg5 = va_arg(args, EXCEPT_ARGN(5, EXCEPT_TERM_HANDLER_TYPE_LIST));
        va_end(args);
        term_handler(status, arg1, arg2, arg3, arg4, arg5);
#elif (EXCEPT_TERM_HANDLER_ARGC == 7)
        va_list args;
        va_start(args, status);
        EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST) arg1 = va_arg(args, EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST) arg2 = va_arg(args, EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST) arg3 = va_arg(args, EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST) arg4 = va_arg(args, EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(5, EXCEPT_TERM_HANDLER_TYPE_LIST) arg5 = va_arg(args, EXCEPT_ARGN(5, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(6, EXCEPT_TERM_HANDLER_TYPE_LIST) arg6 = va_arg(args, EXCEPT_ARGN(6, EXCEPT_TERM_HANDLER_TYPE_LIST));
        va_end(args);
        term_handler(status, arg1, arg2, arg3, arg4, arg5, arg6);
#elif (EXCEPT_TERM_HANDLER_ARGC == 8)
        va_list args;
        va_start(args, status);
        EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST) arg1 = va_arg(args, EXCEPT_ARGN(1, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST) arg2 = va_arg(args, EXCEPT_ARGN(2, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST) arg3 = va_arg(args, EXCEPT_ARGN(3, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST) arg4 = va_arg(args, EXCEPT_ARGN(4, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(5, EXCEPT_TERM_HANDLER_TYPE_LIST) arg5 = va_arg(args, EXCEPT_ARGN(5, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(6, EXCEPT_TERM_HANDLER_TYPE_LIST) arg6 = va_arg(args, EXCEPT_ARGN(6, EXCEPT_TERM_HANDLER_TYPE_LIST));
        EXCEPT_ARGN(7, EXCEPT_TERM_HANDLER_TYPE_LIST) arg7 = va_arg(args, EXCEPT_ARGN(7, EXCEPT_TERM_HANDLER_TYPE_LIST));
        va_end(args);
        term_handler(status, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
#else
        term_handler(status);
#endif
    exit(status);
}

static void stack_tss_free(void* ptr)
{
    free(ptr);
    ptr = NULL;
}

static void last_exception_what_tss_free(void* ptr)
{
    stack_tss_free(ptr);
}

static void stack_tss_create(void)
{
    if (TSS_CREATE(&stack, stack_tss_free) != THRD_SUCCESS)
    {
        fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR:" P_RESET " Failed to create exception stack.\n");
        exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);
    }
}

static void last_exception_what_tss_create(void)
{
    if (TSS_CREATE(&last_exception_what, last_exception_what_tss_free) != THRD_SUCCESS)
    {
        fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR:" P_RESET " Failed to create WHAT buffer.\n");
        exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);
    }
}

EXCEPT_API
void exC_thrd_deinit(void)
{
    // Deallocate the stack and the WHAT buffer (only for the current thread)
    stack_tss_free(TSS_GET(stack));
    last_exception_what_tss_free(TSS_GET(last_exception_what));
}

EXCEPT_API
void exC_global_deinit(void)
{
    // Deallocate the stack and the WHAT buffer (for all threads, i.e. deallocate thread-specific data)
    TSS_DELETE(stack);
    TSS_DELETE(last_exception_what);
}