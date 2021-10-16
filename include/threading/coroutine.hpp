//
// Created by nikon on 5/3/21.
//

#ifndef COROUTINES_COROUTINES_HPP
#define COROUTINES_COROUTINES_HPP

#include <ucontext.h>

namespace threading
{

typedef struct coro_t_ coro_t;
typedef int (*coro_function_t)(coro_t *coro);

/*
 * Coroutine handler.
 */
struct coro_t_
{
    coro_function_t function;
    ucontext_t      suspend_context;
    ucontext_t      resume_context;
    int             yield_value;
    bool            is_coro_finished;
};

/*
 * Coroutine APIs for users/
 */
coro_t *coro_new(coro_function_t function);

int coro_resume(coro_t *coro);

void coro_yield(coro_t *coro, int value);

void coro_free(coro_t *coro);

} // namespace threading

#endif //COROUTINES_COROUTINES_HPP
