#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <queue>
#include <ranges>
#include <thread>
#include <unordered_map>
#include <vector>

#include "semaphore.hpp"
#include "utility/range.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// TODO
// - Compute average duration of task execution after completion of each task in a scalable way.

namespace threading
{

// Names
const std::string ThreadPoolLoggerName = "ThreadPoolLogger";

// Timing
using namespace std::chrono_literals;

class ThreadPool : public std::enable_shared_from_this<ThreadPool>
{
private:
    /* This class is used as a wrapper for packaged_task
     * to detect start/end of the routine, collect and store some metrics, assign priority, etc...
     */
    template <typename F, typename... Args>
    struct Job
    {
        using packaged_task_type = std::packaged_task<typename std::result_of<F(Args...)>::type()>;

        /* Used to store packaged_task */
        std::shared_ptr<packaged_task_type> m_task;

        /* Absolute execution time */
        std::chrono::duration<double, std::micro> m_ms;

        /* Each routine has priority */
        size_t priority{1};

        void operator()()
        {
            const auto start = std::chrono::high_resolution_clock::now();
            (*m_task)();
            const auto end = std::chrono::high_resolution_clock::now();
            m_ms = end - start;
            spdlog::get(ThreadPoolLoggerName)
                ->debug("Finished task: {duration=" + std::to_string(m_ms.count()) + "ms}");
        }
    };

public:
    explicit ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency())
        : m_num_threads(num_threads)
        , m_threads_sem(0)
        , m_tasks_sem(1)
        , m_terminate(false)
    {
        /* Initialize logger for threadpool object */
        spdlog::stdout_color_mt(ThreadPoolLoggerName);

        /* Handle the case when 0 passed as a @num_threads */
        num_threads = std::max(std::size_t(1), num_threads);
        for (; num_threads != 0; num_threads--)
        {
            m_threads.emplace_back(std::thread([this] { this->Task(this); }));
        }
    }

    ~ThreadPool()
    {
        m_terminate = true;
        for (auto _ : range(m_num_threads))
        {
            (void)_;
            m_threads_sem.Signal();
        }

        for (auto& thread : m_threads)
        {
            thread.join();
        }
    }

    template <typename F, typename... Args>
    auto Submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        auto job = std::make_shared<Job<F, Args...>>();
        job->m_task =
            std::make_shared<std::packaged_task<typename std::result_of<F(Args...)>::type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        auto task_result = job->m_task->get_future();
        auto job_wrapper = [job] { (*job)(); };

        m_tasks_sem.Wait();
        m_tasks.emplace(job_wrapper);
        m_tasks_sem.Signal();

        m_threads_sem.Signal();
        return task_result;
    }

private:
    void Task(ThreadPool* thread_pool)
    {
        while (true)
        {
            thread_pool->m_threads_sem.Wait();

            if (thread_pool->m_terminate && m_tasks.empty())
            {
                break;
            }

            thread_pool->m_tasks_sem.Wait();
            auto task = std::move(thread_pool->m_tasks.front());
            thread_pool->m_tasks.pop();
            thread_pool->m_tasks_sem.Signal();

            task();

            if (thread_pool->m_terminate && m_tasks.empty())
            {
                break;
            }
        }
    }

private:
    const std::size_t m_num_threads;

    /* Thread Handling */
    std::vector<std::thread> m_threads;
    Semaphore m_threads_sem;

    /* Task Handling */
    std::queue<std::function<void()>> m_tasks;
    Semaphore m_tasks_sem;

    /* To control task termination */
    bool m_terminate;
};

} // namespace threading

#endif
