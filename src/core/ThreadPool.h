#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include "Defines.h"

/// @brief Simple thread pool class
class ThreadPool {
public:
    /// @brief Set threads count and number of thread handles
    explicit ThreadPool(const unsigned tCount) : workers(tCount), threadsCount(tCount) {}

    ThreadPool() = delete;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /// @brief Creates the threads
    void start() {
        Assert(!running && "Can't start ThreadPool if it's already running");
        running = true;
        for (auto& worker : workers) {
            worker = std::thread(&workerBase, this);
        }
    }

    /// @brief Waits for all tasks to complete, then destroys all threads by joining them
    void stop() {
        Assert(running && "Can't stop ThreadPool if it's not running");
        running = false;
        workersCv.notify_all();
        std::for_each(workers.begin(), workers.end(), std::mem_fn(&std::thread::join));
        workers.clear();
    }

    /// @brief Wait for all tasks in the queue to complete
    void completeTasks() {
        for (;;) {
            if (numTasks == 0) {
                break;
            }
            std::this_thread::yield();
        }
    }

    /// @brief Submit a function with zero or more arguments, and no return value, into the tasks
    /// queue. Notifies thread when the function is submitted
    /// @tparam F The type of the function
    /// @tparam ...Args The types of the arguments
    /// @param task The function to submit to the tasks queue
    /// @param ...args The arguments to pass to the function @task
    template <typename F, typename... Args>
    void scheduleTask(F&& task, Args&&... args) {
        {
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasksQueue.emplace(std::bind(std::forward<F>(task), std::forward<Args>(args)...));
        }
        ++numTasks;
        workersCv.notify_one();
    }

private:
    /// @brief A base function to be assigned to each thread. Waits until it is notified by
    /// scheduleTask() that a task is available, and then retrieves the task from the queue and
    /// executes it
    void workerBase() {
        for (;;) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(tasksMutex);
                workersCv.wait(lock, [this] { return !tasksQueue.empty() || !running; });
                if (!running)
                    return;
                task = std::move(tasksQueue.front());
                tasksQueue.pop();
            }
            task();
            --numTasks;
        }
    }

private:
    /// @brief Thread handles
    std::vector<std::thread> workers{};

    /// @brief A mutex to synchronize access to tasks queue by different threads
    mutable std::mutex tasksMutex{};

    /// @brief A queue of tasks to be executed by the workers
    std::queue<std::function<void()>> tasksQueue{};

    /// @brief A condition variable used to notify worker that a new task has become available
    std::condition_variable workersCv{};

    /// @brief An atomic bool variable indicating if workers should quit
    std::atomic_bool running = false;

    /// @brief Number of tasks in the queue to execute
    std::atomic_size_t numTasks{};

    /// @brief Number of threads
    const unsigned threadsCount{};
};

#endif  // !THREADPOOL_H
