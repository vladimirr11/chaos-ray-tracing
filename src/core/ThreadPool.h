#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>
#include "Defines.h"
#include "Statistics.h"

/// @brief Helps to measure the active running time for each thread when work is assigned
static thread_local bool threadBeginWork = false;

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
        shouldCompleteTasks = true;
        for (;;) {
            if (numTasks == 0 && activeWorkers == 0) {
                shouldCompleteTasks = false;
                break;
            }
            std::this_thread::yield();
        }
    }

    /// @brief Divides 2D loop into _tileSize_ 2D chunks of work. Only the last chunks
    /// per dimension could be with different sizes
    /// @tparam F The type of the function
    /// @param task The function to submit to the tasks queue
    /// @param loopWidth The second dimension of the loop
    /// @param loopHeight The first dimension of the loop
    /// @param tileSize The amount of work per dimension for thread
    template <typename F>
    void parallelLoop2D(F&& task, const size_t loopWidth, const size_t loopHeight,
                        const size_t tileSize) {
        using std::min;
        for (size_t y0 = 0, y1 = tileSize; y0 < y1;
             y0 = y0 + tileSize, y1 = min(y1 + tileSize, loopHeight)) {
            for (size_t x0 = 0, x1 = tileSize; x0 < x1;
                 x0 = x0 + tileSize, x1 = min(x1 + tileSize, loopWidth)) {
                scheduleTask(std::forward<F>(task), x0, x1, y0, y1);
            }
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
                if (shouldCompleteTasks && tasksQueue.empty()) {
                    threadBeginWork = false;
                    reportThreadStats();
                    --activeWorkers;
                }
                workersCv.wait(lock, [this] { return !tasksQueue.empty() || !running; });
                if (!running)
                    return;
                task = std::move(tasksQueue.front());
                tasksQueue.pop();
                lock.unlock();
                if (!threadBeginWork) {
                    ++activeWorkers;
                    threadEntryPoint();
                }
                threadBeginWork = true;
                task();
                --numTasks;
            }
        }
    }

private:
    /// @brief Thread handles
    std::vector<std::thread> workers{};

    /// @brief Mutex to synchronize access to tasks queue by different threads
    mutable std::mutex tasksMutex{};

    /// @brief Queue of tasks to be executed by the workers
    std::queue<std::function<void()>> tasksQueue{};

    /// @brief Condition variable used to notify worker that a new task has become available
    std::condition_variable workersCv{};

    /// @brief Atomic bool variable indicating if workers should quit
    std::atomic_bool running = false;

    /// @brief Boolean flag indicating that tasks have been issued and
    /// the main thread waits for their completion
    std::atomic_bool shouldCompleteTasks = false;

    /// @brief The number of threads that have assigned work
    std::atomic_size_t activeWorkers{};

    /// @brief Number of tasks in the queue
    std::atomic_size_t numTasks{};

    /// @brief Number of threads
    const unsigned threadsCount{};
};

#endif  // !THREADPOOL_H
