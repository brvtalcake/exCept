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

#ifndef EXCEPT_H
#define EXCEPT_H

// TODO: Implement a STACK_TRACE macro

#include "exCept_user_config.h"

#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

#include <metalang99.h>

#include <chaos/preprocessor.h>

#if defined(__cplusplus)
    extern "C" {
#endif

#if defined(P_RED)
    #undef P_RED
#endif
#define P_RED "\033[0;31m"

#if defined(P_BOLD)
    #undef P_BOLD
#endif
#define P_BOLD "\033[1m"

#if defined(P_RESET)
    #undef P_RESET
#endif
#define P_RESET "\033[0m"

#if defined(EXCEPT_COMPILING_ON_WINDOWS) || defined(EXCEPT_COMPILING_ON_LINUX) || defined(EXCEPT_COMPILING_ON_MACOS)
    #undef EXCEPT_COMPILING_ON_WINDOWS
    #undef EXCEPT_COMPILING_ON_LINUX
    #undef EXCEPT_COMPILING_ON_MACOS
#endif
#if defined(_WIN32) || defined(_WIN64)
    #define EXCEPT_COMPILING_ON_WINDOWS 1
#elif defined(__linux__)
    #define EXCEPT_COMPILING_ON_LINUX 1
#elif defined(__APPLE__)
    #define EXCEPT_COMPILING_ON_MACOS 1
#else
    #warning "Unknown platform. Assuming Linux."
    #define EXCEPT_COMPILING_ON_LINUX 1
#endif

#if defined(EXCEPT_API)
    #warning "EXCEPT_API is already defined. Undefining it."
    #undef EXCEPT_API
#endif
#if defined(EXCEPT_COMPILING_ON_WINDOWS)
    #if defined(EXCEPT_BUILDING_DLL)
        #define EXCEPT_API __declspec(dllexport)
    #else
        #define EXCEPT_API __declspec(dllimport)
    #endif
#elif defined(EXCEPT_COMPILING_ON_LINUX) || defined(EXCEPT_COMPILING_ON_MACOS)
    #if (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
        #define EXCEPT_API __attribute__((visibility("default")))
    #else
        #define EXCEPT_API
    #endif
#else
    #if (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
        #define EXCEPT_API __attribute__((visibility("default")))
    #else
        #define EXCEPT_API
    #endif
#endif

#if defined(EXCEPT_ARGC) || defined(EXCEPT_ARGC_PRIVATE)
    #undef EXCEPT_ARGC
    #undef EXCEPT_ARGC_PRIVATE
#endif

// These macros are taken from Jens Gustedt's blog post: https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/
#if 1
#define _ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define HAS_COMMA(...) _ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define _TRIGGER_PARENTHESIS_(...) ,
 
#define ISEMPTY(...)                                                    \
_ISEMPTY(                                                               \
          /* test if there is just one argument, eventually an empty    \
             one */                                                     \
          HAS_COMMA(__VA_ARGS__),                                       \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument   \
             adds a comma */                                            \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
          /* test if the argument together with a parenthesis           \
             adds a comma */                                            \
          HAS_COMMA(__VA_ARGS__ (/*empty*/)),                           \
          /* test if placing it between _TRIGGER_PARENTHESIS_ and the   \
             parenthesis adds a comma */                                \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/))      \
          )
 
#define PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define _ISEMPTY(_0, _1, _2, _3) HAS_COMMA(PASTE5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001 ,
#endif

#if 0
#define EXCEPT_ARGC(...) EXCEPT_ARGC_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define EXCEPT_ARGC_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _count, ...) _count
#else
#define EXCEPT_ARGC(...)                                        \
    CHAOS_PP_EXPR(                                              \
        CHAOS_PP_VARIADIC_IF(ISEMPTY(__VA_ARGS__))              \
            (0)                                                 \
            (CHAOS_PP_VARIADIC_SIZE(__VA_ARGS__))               \
    )
#endif

#if defined(EXCEPT_FIRST_ARG) || defined(EXCEPT_FIRST_ARG_PRIVATE)
    #undef EXCEPT_FIRST_ARG
    #undef EXCEPT_FIRST_ARG_PRIVATE
#endif
#define EXCEPT_FIRST_ARG(...) EXCEPT_FIRST_ARG_PRIVATE(__VA_ARGS__, 0)
#define EXCEPT_FIRST_ARG_PRIVATE(_first, ...) _first

#if !defined(EXCEPT_EXCEPTION_TYPE)
    #define EXCEPT_EXCEPTION_TYPE unsigned int
#endif
#if !defined(EXCEPT_TYPEOF)
    #define EXCEPT_TYPEOF(_var) __typeof__(_var)
#endif
#if !defined(EXCEPT_TERM_HANDLER_SIG)
    typedef void (*term_handler_t)(int);
    #define EXCEPT_TERM_HANDLER_SIG term_handler_t
    #if !defined(EXCEPT_TERM_HANDLER_TYPE_LIST)
        #define EXCEPT_TERM_HANDLER_TYPE_LIST int
    #else
        #error "EXCEPT_TERM_HANDLER_TYPE_LIST is defined but EXCEPT_TERM_HANDLER_SIG is not defined."
    #endif
    #if !defined(TERMINATE_DEFAULT_ERROR_ARGS)
        #define TERMINATE_DEFAULT_ERROR_ARGS EXIT_FAILURE
    #else
        #error "TERMINATE_DEFAULT_ERROR_ARGS is defined but EXCEPT_TERM_HANDLER_SIG is not defined."
    #endif
#else
    #if !defined(EXCEPT_TERM_HANDLER_TYPE_LIST)
        #error "EXCEPT_TERM_HANDLER_SIG is defined but EXCEPT_TERM_HANDLER_TYPE_LIST is not defined."
    #endif
    #if !defined(TERMINATE_DEFAULT_ERROR_ARGS)
        #error "EXCEPT_TERM_HANDLER_SIG is defined but TERMINATE_DEFAULT_ERROR_ARGS is not defined."
    #endif
    typedef EXCEPT_TYPEOF(EXCEPT_TERM_HANDLER_SIG) term_handler_t;
#endif
#if defined(EXCEPT_TERM_HANDLER_ARGC)
    #undef EXCEPT_TERM_HANDLER_ARGC
#endif
#define EXCEPT_TERM_HANDLER_ARGC EXCEPT_ARGC(EXCEPT_TERM_HANDLER_TYPE_LIST)
static_assert(EXCEPT_TERM_HANDLER_ARGC >= 1, "EXCEPT_TERM_HANDLER_TYPE_LIST must be greater than or equal to 1.");
static_assert(EXCEPT_TERM_HANDLER_ARGC <= 8, "EXCEPT_TERM_HANDLER_TYPE_LIST must be less than or equal to 8.");

#if defined(EXCEPT_CAT) || defined(EXCEPT_CAT_PRIMITIVE)
    #undef EXCEPT_CAT
    #undef EXCEPT_CAT_PRIMITIVE
#endif
#define EXCEPT_CAT_PRIMITIVE(x, y) x##y
#define EXCEPT_CAT(x, y) EXCEPT_CAT_PRIMITIVE(x, y)

#if defined(EXCEPT_ARGN)
    #undef EXCEPT_ARGN
#endif
#define EXCEPT_ARGN(n, ...) ML99_EVAL(ML99_listGet(v(n), ML99_list(v(__VA_ARGS__))))

#if defined(EXCEPT_SUB)
    #undef EXCEPT_SUB
#endif
#define EXCEPT_SUB(x, y) ML99_EVAL(ML99_sub(v(x), v(y)))

#if defined(EXCEPT_NAMESPACE)
    #warning "EXCEPT_NAMESPACE is already defined. Undefining it."
    #undef EXCEPT_NAMESPACE
#endif
#define EXCEPT_NAMESPACE(_id) EXCEPT_CAT(exC_, _id)

#if defined(EXCEPT_SYNC_CHANGES) || defined(EXCEPT_SYNC_CHANGES_PRIVATE) || defined(EXCEPT_SYNC_CHANGES_PRIVATE_IMPL) || defined(EXCEPT_SYNC_CHANGES_PRIVATE_ARITY)
    #undef EXCEPT_SYNC_CHANGES
    #undef EXCEPT_SYNC_CHANGES_PRIVATE
    #undef EXCEPT_SYNC_CHANGES_PRIVATE_IMPL
    #undef EXCEPT_SYNC_CHANGES_PRIVATE_ARITY
#endif
#define EXCEPT_SYNC_CHANGES_PRIVATE_IMPL(_var_to_save) v(volatile EXCEPT_TYPEOF(_var_to_save) EXCEPT_NAMESPACE(EXCEPT_CAT(saved_var_, _var_to_save)) = _var_to_save;)
#define EXCEPT_SYNC_CHANGES_PRIVATE_ARITY 1
#define EXCEPT_SYNC_CHANGES(...) ML99_EVAL(ML99_call(ML99_variadicsForEach, v(EXCEPT_SYNC_CHANGES_PRIVATE), v(__VA_ARGS__)))

#if defined(EXCEPT_LOAD) || defined(EXCEPT_LOAD_PRIVATE) || defined(EXCEPT_LOAD_PRIVATE_IMPL) || defined(EXCEPT_LOAD_PRIVATE_ARITY)
    #undef EXCEPT_LOAD
    #undef EXCEPT_LOAD_PRIVATE
    #undef EXCEPT_LOAD_PRIVATE_IMPL
    #undef EXCEPT_LOAD_PRIVATE_ARITY
#endif
#define EXCEPT_LOAD_PRIVATE_IMPL(_var_to_load) v(_var_to_load = EXCEPT_NAMESPACE(EXCEPT_CAT(saved_var_, _var_to_load));)
#define EXCEPT_LOAD_PRIVATE_ARITY 1
#define EXCEPT_LOAD(...) ML99_EVAL(ML99_call(ML99_variadicsForEach, v(EXCEPT_LOAD_PRIVATE), v(__VA_ARGS__)))

#if defined(EXCEPT_SAVE) || defined(EXCEPT_SAVE_PRIVATE) || defined(EXCEPT_SAVE_PRIVATE_IMPL) || defined(EXCEPT_SAVE_PRIVATE_ARITY)
    #undef EXCEPT_SAVE
    #undef EXCEPT_SAVE_PRIVATE
    #undef EXCEPT_SAVE_PRIVATE_IMPL
    #undef EXCEPT_SAVE_PRIVATE_ARITY
#endif
#define EXCEPT_SAVE_PRIVATE_IMPL(_var_to_save) v(EXCEPT_NAMESPACE(EXCEPT_CAT(saved_var_, _var_to_save)) = _var_to_save;)
#define EXCEPT_SAVE_PRIVATE_ARITY 1
#define EXCEPT_SAVE(...) ML99_EVAL(ML99_call(ML99_variadicsForEach, v(EXCEPT_SAVE_PRIVATE), v(__VA_ARGS__)))

#if defined(EXCEPT_TRY_WITH_ARG) || defined(EXCEPT_CATCH) || defined(EXCEPT_THROW) || defined(EXCEPT_FINALLY) || defined(EXCEPT_END_TRY) || defined(EXCEPT_RETHROW) || defined(EXCEPT_VAR) || defined(EXCEPT_CATCH_NUM) || defined(EXCEPT_CATCH_UNNAMED) || defined(EXCEPT_CATCH_NAMED_VAR) || defined(EXCEPT_TERMINATE) || defined(NOEXCEPT) || defined(END_NOEXCEPT) || defined(EXCEPT_TRY) || defined(EXCEPT_WHAT)
    #warning "One or most of EXCEPT_TRY_WITH_ARG, EXCEPT_CATCH, EXCEPT_THROW, EXCEPT_FINALLY, EXCEPT_END_TRY, EXCEPT_RETHROW, EXCEPT_VAR, EXCEPT_CATCH_NUM, EXCEPT_CATCH_UNNAMED, EXCEPT_CATCH_NAMED_VAR, EXCEPT_TERMINATE, NOEXCEPT, END_NOEXCEPT, EXCEPT_TRY and EXCEPT_WHAT are already defined. Undefining them."
    #undef EXCEPT_TRY_WITH_ARG
    #undef EXCEPT_CATCH
    #undef EXCEPT_CATCH_NUM
    #undef EXCEPT_CATCH_UNNAMED
    #undef EXCEPT_CATCH_NAMED_VAR
    #undef EXCEPT_THROW
    #undef EXCEPT_FINALLY
    #undef EXCEPT_END_TRY
    #undef EXCEPT_RETHROW
    #undef EXCEPT_VAR
    #undef EXCEPT_TERMINATE
    #undef NOEXCEPT
    #undef END_NOEXCEPT
    #undef EXCEPT_TRY
    #undef EXCEPT_WHAT
#endif

// TODO: Modify macros
#define EXCEPT_TRY_WITH_ARG(_nesting_lvl)                                         \
    do                                                                            \
    {                                                                             \
        if (!exC_is_global_setup_done() || !exC_is_thread_setup_done())           \
        {                                                                         \
            fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR: " P_RESET                 \
                            "exC_global_setup and/or exC_thread_setup have not "  \
                            "been called. Please call them before using any of "  \
                            "the macros provided by exCept.h.\n");                \
            exit(EXIT_FAILURE);                                                   \
        }                                                                         \
        jmp_buf EXCEPT_NAMESPACE(EXCEPT_CAT(env, _nesting_lvl));                  \
        if (exC_push_stack(&EXCEPT_NAMESPACE(EXCEPT_CAT(env, _nesting_lvl))) != 0)\
        {                                                                         \
            fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR: " P_RESET                 \
                            "exC_push_stack failed. Please check that the stack " \
                            "has been created and that the stack size is "        \
                            "sufficient.\n");                                     \
            exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);                          \
        }                                                                         \
        switch (setjmp(EXCEPT_NAMESPACE(EXCEPT_CAT(env, _nesting_lvl))))          \
        {                                                                         \
            case 0:                                                               \
                {

#if defined(__COUNTER__) && !defined(__INTELLISENSE__)
#define EXCEPT_TRY                                                                  \
    do                                                                              \
    {                                                                               \
        if (!exC_is_global_setup_done() || !exC_is_thread_setup_done())             \
        {                                                                           \
            fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR: " P_RESET                   \
                            "exC_global_setup and/or exC_thread_setup have not "    \
                            "been called. Please call them before using any of "    \
                            "the macros provided by exCept.h.\n");                  \
            exit(EXIT_FAILURE);                                                     \
        }                                                                           \
        jmp_buf EXCEPT_NAMESPACE(EXCEPT_CAT(env, __COUNTER__));                     \
        if (exC_push_stack(                                                         \
            &EXCEPT_NAMESPACE(EXCEPT_CAT(env, EXCEPT_SUB(__COUNTER__, 1)))) != 0)   \
        {                                                                           \
            fprintf(stderr, P_RED P_BOLD "EXCEPT ERROR: " P_RESET                   \
                            "exC_push_stack failed. Please check that the stack "   \
                            "has been created and that the stack size is "          \
                            "sufficient.\n");                                       \
            exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);                            \
        }                                                                           \
        switch (                                                                    \
            setjmp(EXCEPT_NAMESPACE(EXCEPT_CAT(env, EXCEPT_SUB(__COUNTER__, 2)))))  \
        {                                                                           \
            case 0:                                                                 \
                {
#elif defined(__LINE__)
#define EXCEPT_TRY                                                                  \
    do{if(!exC_is_global_setup_done()||!exC_is_thread_setup_done()){fprintf(stderr,P_RED P_BOLD "EXCEPT ERROR: " P_RESET "exC_global_setup and/or exC_thread_setup have not been called. Please call them before using any of the macros provided by exCept.h.\n");exit(EXIT_FAILURE);}jmp_buf EXCEPT_NAMESPACE(EXCEPT_CAT(env,__LINE__));if(exC_push_stack(&EXCEPT_NAMESPACE(EXCEPT_CAT(env,__LINE__)))!=0){fprintf(stderr,P_RED P_BOLD "EXCEPT ERROR: " P_RESET "exC_push_stack failed. Please check that the stack has been created and that the stack size is sufficient.\n");exC_terminate(TERMINATE_DEFAULT_ERROR_ARGS);}switch(setjmp(EXCEPT_NAMESPACE(EXCEPT_CAT(env,__LINE__)))){case 0:{
#else
    #error "Neither __COUNTER__ nor __LINE__ are defined. Cannot use EXCEPT_TRY."
#endif

#define EXCEPT_CATCH_NUM(x)      \
                }                \
                exC_pop_stack(); \
                break;           \
            case x:              \
                {

#define EXCEPT_CATCH_UNNAMED     \
                }                \
                exC_pop_stack(); \
                break;           \
            default:             \
                {

#define EXCEPT_CATCH_NAMED_VAR(_var)                                \
                }                                                   \
                exC_pop_stack();                                    \
                break;                                              \
            default:                                                \
                EXCEPT_EXCEPTION_TYPE _var = exC_last_exception();  \
                {

#define EXCEPT_CATCH(...) \
    CHAOS_PP_VARIADIC_IF(CHAOS_PP_EQUAL(EXCEPT_ARGC(__VA_ARGS__), 1))               \
    (                                                                               \
        CHAOS_PP_VARIADIC_IF(CHAOS_PP_IS_NUMERIC(EXCEPT_FIRST_ARG(__VA_ARGS__)))    \
        (                                                                           \
            EXCEPT_CATCH_NUM(EXCEPT_FIRST_ARG(__VA_ARGS__))                         \
        )                                                                           \
        (                                                                           \
            EXCEPT_CATCH_NAMED_VAR(EXCEPT_FIRST_ARG(__VA_ARGS__))                   \
        )                                                                           \
    )                                                                               \
    (                                                                               \
        EXCEPT_CATCH_UNNAMED                                                        \
    )

#include <stddef.h>

#if defined(EXCEPT_ARG_1_AND_2)
    #undef EXCEPT_ARG_1_AND_2
#endif
#define EXCEPT_ARG_1_AND_2(_1, _2, ...) _1, _2
#define EXCEPT_THROW(...) exC_unwind(EXCEPT_ARG_1_AND_2(__VA_ARGS__, NULL), NULL)

#define EXCEPT_END_TRY           \
                }                \
                exC_pop_stack(); \
                break;           \
        }                        \
    } while (0)

#define EXCEPT_RETHROW exC_unwind(exC_last_exception(), exC_last_exception_what(), NULL)

#define EXCEPT_VAR(_var) EXCEPT_NAMESPACE(EXCEPT_CAT(saved_var_, _var))

#define EXCEPT_FINALLY

#define EXCEPT_TERMINATE(status, ...) exC_terminate(status, __VA_ARGS__)

#define NOEXCEPT EXCEPT_TRY_WITH_ARG(EXCEPT_CAT(__LINE__, __COUNTER__)) {

#define END_NOEXCEPT } EXCEPT_CATCH_UNNAMED { EXCEPT_TERMINATE(TERMINATE_DEFAULT_ERROR_ARGS); } EXCEPT_END_TRY;

#define EXCEPT_WHAT exC_last_exception_what()

#if defined(EXCEPT_LOWERCASE)
    #if defined(try) || defined(catch) || defined(throw) || defined(finally) || defined(end_try) || defined(rethrow) || defined(load) || defined(sync_changes) || defined(save) || defined(var) || defined(terminate) || defined(noexcept) || defined(end_noexcept) || defined(what)
        #warning "One or most of try, catch, throw, finally, end_try, rethrow, load, sync_changes, save, var, terminate, noexcept, end_noexcept and what are already defined. Undefining them."
        #undef try
        #undef catch
        #undef throw
        #undef finally
        #undef end_try
        #undef rethrow
        #undef load
        #undef sync_changes
        #undef save
        #undef var
        #undef terminate
        #undef noexcept
        #undef end_noexcept
        #undef what
    #endif
    #define try EXCEPT_TRY
    #define catch(...) EXCEPT_CATCH(__VA_ARGS__)
    #define throw(...)                                                                                                  \
    {                                                                                                                   \
        static_assert(                                                                                                  \
            EXCEPT_ARGC(__VA_ARGS__) == 2 ||                                                                            \
            EXCEPT_ARGC(__VA_ARGS__) == 1 ||                                                                            \
            EXCEPT_ARGC(__VA_ARGS__) == 0,                                                                              \
            "throw takes 0, 1 or 2 arguments.");                                                                        \
    }                                                                                                                   \
    ML99_EVAL(ML99_if(ML99_or(ML99_natEq(v(EXCEPT_ARGC(__VA_ARGS__)), v(2)), ML99_natEq(v(EXCEPT_ARGC(__VA_ARGS__)), v(1))), v(EXCEPT_THROW(__VA_ARGS__)), v(EXCEPT_RETHROW)))
    #define finally EXCEPT_FINALLY
    #define end_try EXCEPT_END_TRY
    #define rethrow EXCEPT_RETHROW
    #define load(...) EXCEPT_LOAD(__VA_ARGS__)
    #define sync_changes(...) EXCEPT_SYNC_CHANGES(__VA_ARGS__)
    #define save(...) EXCEPT_SAVE(__VA_ARGS__)
    #define var(...) EXCEPT_VAR(__VA_ARGS__)
    #define terminate(status, ...) EXCEPT_TERMINATE(status, __VA_ARGS__)
    #define noexcept NOEXCEPT
    #define end_noexcept END_NOEXCEPT
    #define what EXCEPT_WHAT
#else
    #if defined(TRY) || defined(CATCH) || defined(THROW) || defined(FINALLY) || defined(END_TRY) || defined(RETHROW) || defined(LOAD) || defined(SYNC_CHANGES) || defined(SAVE) || defined(VAR) || defined(TERMINATE) || defined(WHAT)
        #warning "One or most of TRY, CATCH, THROW, FINALLY, END_TRY, RETHROW, LOAD, SYNC_CHANGES, SAVE, VAR, TERMINATE and WHAT are already defined. Undefining them."
        #undef TRY
        #undef CATCH
        #undef THROW
        #undef FINALLY
        #undef END_TRY
        #undef RETHROW
        #undef LOAD
        #undef SYNC_CHANGES
        #undef SAVE
        #undef VAR
        #undef TERMINATE
        #undef WHAT
    #endif
    #define TRY EXCEPT_TRY
    #define CATCH(...) EXCEPT_CATCH(__VA_ARGS__)
    #define THROW(...)                                                                                                  \
    {                                                                                                                   \
        static_assert(                                                                                                  \
            EXCEPT_ARGC(__VA_ARGS__) == 2 ||                                                                            \
            EXCEPT_ARGC(__VA_ARGS__) == 1 ||                                                                            \
            EXCEPT_ARGC(__VA_ARGS__) == 0,                                                                              \
            "THROW takes 0, 1 or 2 arguments.");                                                                        \
    }                                                                                                                   \
    ML99_EVAL(ML99_if(ML99_or(ML99_natEq(v(EXCEPT_ARGC(__VA_ARGS__)), v(2)), ML99_natEq(v(EXCEPT_ARGC(__VA_ARGS__)), v(1))), v(EXCEPT_THROW(__VA_ARGS__, NULL)), v(EXCEPT_RETHROW)))
    #define FINALLY EXCEPT_FINALLY
    #define END_TRY EXCEPT_END_TRY
    #define RETHROW EXCEPT_RETHROW
    #define LOAD(...) EXCEPT_LOAD(__VA_ARGS__)
    #define SYNC_CHANGES(...) EXCEPT_SYNC_CHANGES(__VA_ARGS__)
    #define SAVE(...) EXCEPT_SAVE(__VA_ARGS__)
    #define VAR(...) EXCEPT_VAR(__VA_ARGS__)
    #define TERMINATE(status, ...) EXCEPT_TERMINATE(status, __VA_ARGS__)
    #define WHAT EXCEPT_WHAT
#endif

#if defined(__cplusplus)
    }
#endif

#if defined(ALWAYS_THROWS)
    #warning "ALWAYS_THROWS is already defined. Undefining it."
    #undef ALWAYS_THROWS
#endif
#if defined(EXCEPT_SENTINEL_NULL)
    #undef EXCEPT_SENTINEL_NULL
#endif
#if defined(EXCEPT_COND_PROB)
    #undef EXCEPT_COND_PROB
#endif
#if defined(STACK_ALLOC) || defined(STACK_FREE)
    #undef STACK_ALLOC
    #undef STACK_FREE
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define STACK_ALLOC(type, var, size, ...)                                           \
        CHAOS_PP_VARIADIC_IF(CHAOS_PP_EQUAL(EXCEPT_ARGC(__VA_ARGS__), 0))               \
        (                                                                               \
            type var = __builtin_alloca(size)                                           \
        )                                                                               \
        (                                                                               \
            type var = __builtin_alloca_with_align(size, EXCEPT_FIRST_ARG(__VA_ARGS__)) \
        )
    #define STACK_FREE(ptr, ...) ((void)ptr)
#elif defined(_MSC_VER)
    #include <malloc.h>
    #define STACK_ALLOC(type, var, size, ...)                                                           \
        CHAOS_PP_VARIADIC_IF(CHAOS_PP_EQUAL(EXCEPT_ARGC(__VA_ARGS__), 0))                               \
        (                                                                                               \
            type var = _malloca(size)                                                                   \
        )                                                                                               \
        (                                                                                               \
            type var = _malloca(size + EXCEPT_FIRST_ARG(__VA_ARGS__));                                  \
            assert(var != NULL);                                                                        \
            uintptr_t EXCEPT_CAT(exC_ALIGN_, EXCEPT_CAT(var, _addr)) = (uintptr_t)var;                  \
            while (EXCEPT_CAT(exC_ALIGN_, EXCEPT_CAT(var, _addr)) % EXCEPT_FIRST_ARG(__VA_ARGS__) != 0) \
            {                                                                                           \
                EXCEPT_CAT(exC_ALIGN_, EXCEPT_CAT(var, _addr))++;                                       \
            }                                                                                           \
            var = (type)EXCEPT_CAT(exC_ALIGN_, EXCEPT_CAT(var, _addr));                                 \
        )
    #define STACK_FREE(ptr, ...) _freea(ptr)
#else
    #include <stdlib.h>
    #define STACK_ALLOC(type, var, size, ...)                               \
        CHAOS_PP_VARIADIC_IF(CHAOS_PP_EQUAL(EXCEPT_ARGC(__VA_ARGS__), 0))   \
        (                                                                   \
            type var = malloc(size)                                         \
        )                                                                   \
        (                                                                   \
            type var = aligned_alloc(EXCEPT_FIRST_ARG(__VA_ARGS__), size)   \
        )
    #define STACK_FREE(ptr, ...) free(ptr)
#endif

#if defined(__GNUC__)
    #define EXCEPT_COND_PROB(expr, eq_to, prob) __builtin_expect_with_probability(!!(expr), (eq_to), (prob))
#elif defined(__clang__) && defined(__has_builtin)
    #if __has_builtin(__builtin_expect_with_probability)
        #define EXCEPT_COND_PROB(expr, eq_to, prob) __builtin_expect_with_probability(!!(expr), (eq_to), (prob))
    #else
        #define EXCEPT_COND_PROB(expr, eq_to, prob) (expr)
    #endif
#else
    #define EXCEPT_COND_PROB(expr, eq_to, prob) (expr)
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define EXCEPT_SENTINEL_NULL(...) __attribute__((__sentinel__(__VA_ARGS__)))
#else
    #define EXCEPT_SENTINEL_NULL(...)
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !(__STDC_VERSION__ >= 202300L) && !defined(__cplusplus)
    #include <stdnoreturn.h>
    #define ALWAYS_THROWS noreturn
#elif defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202300L)
    #define ALWAYS_THROWS [[noreturn]]
#else
    #if defined(__GNUC__) || defined(__clang__)
        #define ALWAYS_THROWS __attribute__((__noreturn__))
    #elif defined(_MSC_VER)
        #define ALWAYS_THROWS __declspec(noreturn)
    #else
        #define ALWAYS_THROWS
    #endif
#endif

#if defined(EXCEPT_NORETURN)
    #undef EXCEPT_NORETURN
#endif
#define EXCEPT_NORETURN ALWAYS_THROWS

// User-usable API
/**
 * @brief Flags that can be passed to `exC_global_setup`.
 * @note Nothing for now but will be used in the future.
 * @details
 * - `FLAG_COUNT` is the number of flags.
 */
enum exC_flags
{
    FLAG_COUNT = 0
};
/**
 * @brief Flags that can be passed to `exC_global_setup`.
 * @note Nothing for now but will be used in the future.
 * @details
 * - `FLAG_COUNT` is the number of flags.
 */
typedef enum exC_flags exC_flags_t;

/**
 * @fn int exC_global_setup(size_t stack_size, exC_flags_t flags)
 * @brief Setup the exception handling system.
 * @note This function must be called before any other function of the exception handling system, and before using any of the provided macros.
 * 
 * @param stack_size The number of possible nested `TRY` blocks.
 * @param flags The flags to use.
 * @return 0 on success, non-0 on failure.
 */
EXCEPT_API int exC_global_setup(size_t stack_size, exC_flags_t flags);

/**
 * @fn int exC_thrd_setup(void)
 * @brief Setup the exception handling system for the calling thread.
 * @note This function must be called for every thread that will use the exception handling system.
 * 
 * @return 0 on success, non-0 on failure.
 */
EXCEPT_API int exC_thrd_setup(void);

/**
 * @fn void exC_thrd_deinit(void)
 * @brief Deinitialize the exception handling system for the calling thread.
 * @note This function should be called for every thread that used the exception handling system.
 */
EXCEPT_API void exC_thrd_deinit(void);

/**
 * @fn void exC_global_deinit(void)
 * @brief Deinitialize the exception handling system.
 * @note This function should be called after every thread that used the exception handling system has been deinitialized.
 */
EXCEPT_API void exC_global_deinit(void);

/**
 * @fn int exC_set_term_handler(term_handler_t handler)
 * @brief Set the termination handler.
 * 
 * @param handler The termination handler to use.
 * @return 0 if the termination handler was successfully set, non-0 otherwise.
 */
EXCEPT_API int exC_set_term_handler(term_handler_t handler);

// Implementer-usable API
/**/
EXCEPT_API                          int  exC_is_global_setup_done(void);
EXCEPT_API                          int  exC_is_thread_setup_done(void);
EXCEPT_API                          int  exC_is_stack_size_set(void);
EXCEPT_API                          int  exC_is_stack_created(void);
EXCEPT_API                          int  exC_push_stack(jmp_buf* env);
EXCEPT_API                         void  exC_pop_stack(void);
EXCEPT_NORETURN EXCEPT_SENTINEL_NULL(0)
EXCEPT_API                         void  exC_unwind(EXCEPT_EXCEPTION_TYPE except, ...);
EXCEPT_API                         char* exC_last_exception_what(void);
EXCEPT_API         EXCEPT_EXCEPTION_TYPE exC_last_exception(void);
EXCEPT_NORETURN
EXCEPT_API                         void  exC_terminate(int status, ...);

#endif // EXCEPT_H

#if !defined(EXCEPT_SOURCE)
// Undefine every macros that are just used in the header and exCept.c (which has to define `EXCEPT_SOURCE`), so user's
// macro namespace is not polluted too much

#undef EXCEPT_COND_PROB
#undef EXCEPT_SENTINEL_NULL
#undef EXCEPT_NORETURN
#undef EXCEPT_API
#undef STACK_ALLOC
#undef STACK_FREE

#endif