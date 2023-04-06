#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <algorithm>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

/// @brief Abstract base functor class that all scenes should inherit
struct ParallelTask {
    virtual void operator()(const size_t loopStart, const size_t loopEnd) = 0;
    virtual ~ParallelTask() {}
};

/// @brief Simple task manager class
class TaskManager {
public:
    /// @brief Set threads count and number of thread handles.
    /// @param tCount The number of threads to use
    explicit TaskManager(const unsigned tCount) : workers(tCount), threadsCount(tCount) {}

    TaskManager() = delete;
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    /// @brief Creates the threads and assign a worker to each thread.
    void start() {
        assert(!running && "Can't start TaskManager if it's already running");
        running = true;
        for (auto& worker : workers) {
            worker = std::thread(&workerBase, this);
        }
    }

    /// @brief Waits for all tasks to complete, then destroys all threads by joining them.
    void stop() {
        assert(running && "Can't stop TaskManager if it's not running");
        running = false;
        workersCv.notify_all();
        std::for_each(workers.begin(), workers.end(), std::mem_fn(&std::thread::join));
        workers.clear();
    }

    /// @brief Parallelize a task by splitting it into blocks. Each block is then submitted to the
    /// available threads.
    /// @tparam T The type of the indices in the loop
    /// @tparam F The type of function to loop through
    /// @param startIdx The first index in the loop
    /// @param endIdx One past the last index in the loop
    /// @param task The function to loop through
    /// @param numBlocks The numbers of blocks to split the task into
    template <typename T, typename F>
    void runParallelFor(const T startIdx, const T endIdx, F&& task, size_t numBlocks = 0) {
        if (numBlocks == 0)
            numBlocks = threadsCount;

        const size_t totalRunDist = (size_t)(endIdx - startIdx);
        const size_t workerRunDist = totalRunDist / numBlocks;
        for (size_t i = 0; i < numBlocks; i++) {
            const size_t workerStartIdx = i * workerRunDist;
            const size_t workerEndIdx = workerStartIdx + workerRunDist;
            scheduleTask(std::forward<F>(task), (T)workerStartIdx, (T)workerEndIdx);
        }
    }

    /// @brief Wait for all tasks in the queue to complete.
    void completeTasks() {
        for (;;) {
            if (numTasks == 0) {
                break;
            }
            std::this_thread::yield();
        }
    }

private:
    /// @brief Submit a function with zero or more arguments, and no return value, into the tasks
    /// queue. Notifies thread when the function is submitted.
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

    /// @brief A base function to be assigned to each thread. Waits until it is notified by
    /// scheduleTask() that a task is available, and then retrieves the task from the queue and
    /// executes it.
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

#endif  // !TASKMANAGER_H
