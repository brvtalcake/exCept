#ifndef EXCEPT_H
#define EXCEPT_H

#include <setjmp.h>
#include <stdlib.h>

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


#if defined(__cplusplus)
    extern "C" {
#endif

EXCEPT_API int  exC_set_stack_size(size_t size);
EXCEPT_API int  exC_is_stack_size_set(void);
EXCEPT_API void exC_create_stack(void);
EXCEPT_API int  exC_is_stack_created(void);
EXCEPT_API void exC_destroy_stack(void);
EXCEPT_API int  exC_push_stack(jmp_buf* env);
EXCEPT_API void exC_pop_stack(void);
EXCEPT_API void exC_rewind(int except);

#if defined(EXCEPT_NAMESPACE)
    #warning "EXCEPT_NAMESPACE is already defined. Undefining it."
    #undef EXCEPT_NAMESPACE
#endif
#define EXCEPT_NAMESPACE(_id) exC_##_id

#if defined(EXCEPT_TRY) || defined(EXCEPT_CATCH) || defined(EXCEPT_THROW) || defined(EXCEPT_FINALLY) || defined(EXCEPT_END_TRY)
    #warning "One or most of EXCEPT_TRY, EXCEPT_CATCH, EXCEPT_THROW, EXCEPT_FINALLY and EXCEPT_END_TRY are already defined. Undefining them."
    #undef EXCEPT_TRY
    #undef EXCEPT_CATCH
    #undef EXCEPT_THROW
    #undef EXCEPT_FINALLY
    #undef EXCEPT_END_TRY
#endif

/*
 * USAGE: 
 * TRY {
 *    // code
 * } CATCH(1) {
 *   // code
 * } CATCH(2) {
 *  // code
 * } FINALLY { // optional
 * // code
 * } END_TRY;
 */

// No `finally` block for now.

#define EXCEPT_TRY                                  \
    do                                              \
    {                                               \
        if (!exC_is_stack_created())                \
        {                                           \
            exC_create_stack();                     \
        }                                           \
        jmp_buf EXCEPT_NAMESPACE(env);              \
        if (exC_push_stack(&EXCEPT_NAMESPACE(env))) \
        {                                           \
            fprintf(stderr, "Stack overflow.\n");   \
            exit(EXIT_FAILURE);                     \
        }                                           \
        switch (setjmp(EXCEPT_NAMESPACE(env)))      \
        {                                           \
            case 0:                                 \
                {

#define EXCEPT_CATCH(x)                             \
                }                                   \
                exC_pop_stack();                    \
                break;                              \
            case x:                                 \
                {

#define EXCEPT_THROW(x) exC_rewind(x)

#define EXCEPT_END_TRY                              \
                }                                   \
                exC_pop_stack();                    \
                break;                              \
            default:                                \
        }                                           \
    } while (0)


#if defined(EXCEPT_LOWERCASE)
    #if defined(try) || defined(catch) || defined(throw) || defined(finally) || defined(end_try)
        #warning "One or most of try, catch, throw, finally and end_try are already defined. Undefining them."
        #undef try
        #undef catch
        #undef throw
        #undef finally
        #undef end_try
    #endif
    #define try EXCEPT_TRY
    #define catch(x) EXCEPT_CATCH(x)
    #define throw(x) EXCEPT_THROW(x)
    #define finally EXCEPT_FINALLY
    #define end_try EXCEPT_END_TRY
#else
    #if defined(TRY) || defined(CATCH) || defined(THROW) || defined(FINALLY) || defined(END_TRY)
        #warning "One or most of TRY, CATCH, THROW, FINALLY and END_TRY are already defined. Undefining them."
        #undef TRY
        #undef CATCH
        #undef THROW
        #undef FINALLY
        #undef END_TRY
    #endif
    #define TRY EXCEPT_TRY
    #define CATCH(x) EXCEPT_CATCH(x)
    #define THROW(x) EXCEPT_THROW(x)
    #define FINALLY EXCEPT_FINALLY
    #define END_TRY EXCEPT_END_TRY
#endif

#if defined(__cplusplus)
    }
#endif

#endif // EXCEPT_H