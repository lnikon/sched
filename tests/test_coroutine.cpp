#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <iostream>
#include <queue>

#include "threading/coroutine.hpp"

using namespace threading;

int hello_world(coro_t* coro)
{
    std::cout << "Hello";
    coro_yield(coro, 1);
    std::cout << " world!";
    return 2;
}

int goodbye_worl(coro_t* coro)
{
    std::cout << "Goodbye";
    coro_yield(coro, 1);
    std::cout << " world@";
    return 2;
}

void scheduler()
{
    std::queue<coro_t*> tasks;
    tasks.emplace(coro_new(hello_world));
    tasks.emplace(coro_new(goodbye_worl));

    while (!tasks.empty())
    {
        auto task = tasks.front();
        tasks.pop();
				const auto rc = coro_resume(task);
        if (rc != -1)
        {
						REQUIRE(rc != -1);
            tasks.emplace(task);
        }
        else
        {
						REQUIRE(task->is_coro_finished);
						REQUIRE(rc == -1);
            coro_free(task);
        }
    }
}

TEST_CASE("Concurrently execute two coroutines", "[Coroutines]")
{
    scheduler();
}
