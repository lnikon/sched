#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <chrono>
#include <iostream>
#include <thread>

#include "threading/thread_pool.hpp"
#include "utility/range.hpp"

#include <spdlog/spdlog.h>

TEST_CASE("Increment counter on thread pool", "[ThreadPool]")
{
		spdlog::set_level(spdlog::level::debug);
    using namespace std::chrono_literals;

    const size_t count{10};
		
		// Simple, non-scalable counter
    struct SafeCounter
    {
        void Incr()
        {
            std::lock_guard<std::mutex> lg(m);
            c++;
        }

        size_t Get()
        {
            return c;
        }

    private:
        std::mutex m;
        size_t c{0};
    };

    auto counter = std::make_shared<SafeCounter>();

    {
        threading::ThreadPool threadPool;
        for (int i : range(count))
        {
            threadPool.Submit([](std::shared_ptr<SafeCounter> counter) { counter->Incr(); },
                              counter);
        }
    }

    REQUIRE(counter->Get() == count);
}
