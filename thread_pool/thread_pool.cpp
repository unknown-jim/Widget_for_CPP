#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <thread>

class threadPoll
{
    struct Pool
    {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;

public:
    threadPoll(size_t count) : pool_(std::make_shared<Pool>())
    {
        for (size_t i = 0; i < count; i += 1)
        {
            std::thread(
                [pool = pool_]
                {
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while (true)
                    {
                        if (!pool->tasks.empty())
                        {
                            auto task = pool->tasks.front();
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        }
                        else if (pool->isClosed) break;
                        else pool->cond.wait(locker);
                    }
                }).detach();
        }
    }

    threadPoll() = default;
    threadPoll(threadPoll&&) = default;

    ~threadPoll() {
        if(pool_) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void addTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }
};