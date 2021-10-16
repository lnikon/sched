#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <chrono>
#include <iostream>
#include <thread>

#include "threading/thread_safe_queue.hpp"

TEST_CASE("Add functions then retreive and execute them", "[ThreadSafeQueue]")
{
    using namespace std::chrono_literals;

    threading::ThreadSafeQueue<std::function<int(void)>> tsq;

    const size_t count{10};

    std::thread th1(
        [&tsq]
        {
            for (size_t i = 0; i <= count; ++i)
            {
                tsq.Enqueue([i]() { return i; });
            }
        });

    std::thread th2(
        [&tsq]
        {
						int sum{0};
            for (size_t i = 0; i <= count; ++i)
            {
                std::function<int(void)> f;
                tsq.Dequeue(f);
                sum += f();
            }
						
						REQUIRE(sum == (count * (count + 1)) / 2);
        });

    th1.join();
    th2.join();
}
