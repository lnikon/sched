#ifndef THREAD_SAFE_QUEUE
#define THREAD_SAFE_QUEUE

#include <condition_variable>
#include <mutex>
#include <queue>

namespace threading
{
template <typename DataType>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    ~ThreadSafeQueue() = default;

    void Enqueue(const DataType& data);
    void Dequeue(DataType& data);

private:
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::queue<DataType> m_queue;
};

template <typename DataType>
void ThreadSafeQueue<DataType>::Enqueue(const DataType& data)
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_queue.push(data);
    m_cv.notify_one();
}

template <typename DataType>
void ThreadSafeQueue<DataType>::Dequeue(DataType& data)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cv.wait(lk, [this] { return !m_queue.empty(); });
    data = std::move(m_queue.front());
    m_queue.pop();
}

} // namespace threading

#endif // THREAD_SAFE_QUEUE
