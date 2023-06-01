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

#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

#include <metalang99.h>

#include <chaos/preprocessor.h>

#if defined(__cplusplus)
    extern "C" {
#endif

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
#define EXCEPT_ARGC(...) EXCEPT_ARGC_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define EXCEPT_ARGC_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _count, ...) _count

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
    #define EXCEPT_TYPEOF(_var) typeof(_var)
#endif

EXCEPT_API                  int  exC_set_stack_size(size_t size);
EXCEPT_API                  int  exC_is_stack_size_set(void);
EXCEPT_API                  void exC_create_stack(void);
EXCEPT_API                  int  exC_is_stack_created(void);
EXCEPT_API                  void exC_destroy_stack(void);
EXCEPT_API                  int  exC_push_stack(jmp_buf* env);
EXCEPT_API                  void exC_pop_stack(void);
EXCEPT_API                  void exC_unwind(int except);
EXCEPT_API EXCEPT_EXCEPTION_TYPE exC_last_exception(void);

#if defined(EXCEPT_CAT) || defined(EXCEPT_CAT_PRIMITIVE)
    #undef EXCEPT_CAT
    #undef EXCEPT_CAT_PRIMITIVE
#endif
#define EXCEPT_CAT_PRIMITIVE(x, y) x##y
#define EXCEPT_CAT(x, y) EXCEPT_CAT_PRIMITIVE(x, y)

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

#if defined(EXCEPT_TRY) || defined(EXCEPT_CATCH) || defined(EXCEPT_THROW) || defined(EXCEPT_FINALLY) || defined(EXCEPT_END_TRY) || defined(EXCEPT_RETHROW) || defined(EXCEPT_VAR) || defined(EXCEPT_CATCH_NUM) || defined(EXCEPT_CATCH_UNNAMED) || defined(EXCEPT_CATCH_NAMED_VAR)
    #warning "One or most of EXCEPT_TRY, EXCEPT_CATCH, EXCEPT_THROW, EXCEPT_FINALLY, EXCEPT_END_TRY, EXCEPT_RETHROW, EXCEPT_VAR, EXCEPT_CATCH_NUM, EXCEPT_CATCH_UNNAMED and EXCEPT_CATCH_NAMED_VAR are already defined. Undefining them."
    #undef EXCEPT_TRY
    #undef EXCEPT_CATCH
    #undef EXCEPT_CATCH_NUM
    #undef EXCEPT_CATCH_UNNAMED
    #undef EXCEPT_CATCH_NAMED_VAR
    #undef EXCEPT_THROW
    #undef EXCEPT_FINALLY
    #undef EXCEPT_END_TRY
    #undef EXCEPT_RETHROW
    #undef EXCEPT_VAR
#endif

#define EXCEPT_TRY(_nesting_lvl)                                                  \
    do                                                                            \
    {                                                                             \
        if (!exC_is_stack_created())                                              \
        {                                                                         \
            exC_create_stack();                                                   \
        }                                                                         \
        jmp_buf EXCEPT_NAMESPACE(EXCEPT_CAT(env, _nesting_lvl));                  \
        if (exC_push_stack(&EXCEPT_NAMESPACE(EXCEPT_CAT(env, _nesting_lvl))) < 0) \
        {                                                                         \
            fprintf(stderr, "Stack overflow.\n");                                 \
            exit(EXIT_FAILURE);                                                   \
        }                                                                         \
        switch (setjmp(EXCEPT_NAMESPACE(EXCEPT_CAT(env, _nesting_lvl))))          \
        {                                                                         \
            case 0:                                                               \
                {

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

#define EXCEPT_THROW(...) exC_unwind(EXCEPT_FIRST_ARG(__VA_ARGS__))

#define EXCEPT_END_TRY           \
                }                \
                exC_pop_stack(); \
                break;           \
        }                        \
    } while (0)

#define EXCEPT_RETHROW exC_unwind(exC_last_exception())

#define EXCEPT_VAR(_var) EXCEPT_NAMESPACE(EXCEPT_CAT(saved_var_, _var))

#define EXCEPT_FINALLY

#if defined(EXCEPT_LOWERCASE)
    #if defined(try) || defined(catch) || defined(throw) || defined(finally) || defined(end_try) || defined(rethrow) || defined(load) || defined(sync_changes) || defined(save) || defined(var)
        #warning "One or most of try, catch, throw, finally, end_try, rethrow, load, sync_changes, save and var are already defined. Undefining them."
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
    #endif
    #define try(lvl) EXCEPT_TRY(lvl)
    #define catch(...) EXCEPT_CATCH(__VA_ARGS__)
    #define throw(...)                                                                                                  \
    {                                                                                                                   \
        static_assert(EXCEPT_ARGC(__VA_ARGS__) == 1 || EXCEPT_ARGC(__VA_ARGS__) == 0, "throw takes 0 or 1 argument.");  \
    }                                                                                                                   \
    ML99_EVAL(ML99_call(ML99_if, ML99_natEq(v(EXCEPT_ARGC(__VA_ARGS__)), v(1)), v(EXCEPT_THROW(__VA_ARGS__)), v(EXCEPT_RETHROW)))
    #define finally EXCEPT_FINALLY
    #define end_try EXCEPT_END_TRY
    #define rethrow EXCEPT_RETHROW
    #define load(...) EXCEPT_LOAD(__VA_ARGS__)
    #define sync_changes(...) EXCEPT_SYNC_CHANGES(__VA_ARGS__)
    #define save(...) EXCEPT_SAVE(__VA_ARGS__)
    #define var(...) EXCEPT_VAR(__VA_ARGS__)
#else
    #if defined(TRY) || defined(CATCH) || defined(THROW) || defined(FINALLY) || defined(END_TRY) || defined(RETHROW) || defined(LOAD) || defined(SYNC_CHANGES) || defined(SAVE) || defined(VAR)
        #warning "One or most of TRY, CATCH, THROW, FINALLY, END_TRY, RETHROW, LOAD, SYNC_CHANGES, SAVE and VAR are already defined. Undefining them."
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
    #endif
    #define TRY(lvl) EXCEPT_TRY(lvl)
    #define CATCH(...) EXCEPT_CATCH(__VA_ARGS__)
    #define THROW(...)                                                                                                  \
    {                                                                                                                   \
        static_assert(EXCEPT_ARGC(__VA_ARGS__) == 1 || EXCEPT_ARGC(__VA_ARGS__) == 0, "THROW takes 0 or 1 argument.");  \
    }                                                                                                                   \
    ML99_EVAL(ML99_call(ML99_if, ML99_natEq(v(EXCEPT_ARGC(__VA_ARGS__)), v(1)), v(EXCEPT_THROW(__VA_ARGS__)), v(EXCEPT_RETHROW)))
    #define FINALLY EXCEPT_FINALLY
    #define END_TRY EXCEPT_END_TRY
    #define RETHROW EXCEPT_RETHROW
    #define LOAD(...) EXCEPT_LOAD(__VA_ARGS__)
    #define SYNC_CHANGES(...) EXCEPT_SYNC_CHANGES(__VA_ARGS__)
    #define SAVE(...) EXCEPT_SAVE(__VA_ARGS__)
    #define VAR(...) EXCEPT_VAR(__VA_ARGS__)
#endif

#if defined(__cplusplus)
    }
#endif

#if defined(ALWAYS_THROWS)
    #warning "ALWAYS_THROWS is already defined. Undefining it."
    #undef ALWAYS_THROWS
#endif
#if __STDC_VERSION__ >= 201112L && !(__STDC_VERSION__ >= 202300L)
    #include <stdnoreturn.h>
    #define ALWAYS_THROWS noreturn
#elif __STDC_VERSION__ >= 202300L
    #define ALWAYS_THROWS [[noreturn]]
#else
    #if defined(__GNUC__) || defined(__clang__)
        #define ALWAYS_THROWS __attribute__((noreturn))
    #elif defined(_MSC_VER)
        #define ALWAYS_THROWS __declspec(noreturn)
    #else
        #define ALWAYS_THROWS
    #endif
#endif

#endif // EXCEPT_H
