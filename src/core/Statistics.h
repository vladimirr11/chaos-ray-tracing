#ifndef STATISTICS_H
#define STATISTICS_H

#include <array>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

enum StatTest { NUM_TRIANGLE_ISECT_TESTS, NUM_TRIANGLE_ISECTS, NUM_TESTS };

class StatRegisterer {
public:
    using DataArray = std::array<int64_t, NUM_TESTS>;
    using CallbacksArray = std::vector<std::function<void()>>;

public:
    StatRegisterer(std::function<void()>);

    static __forceinline DataArray& data() { return statsData; }

    static void invokeCallbacks() {
        for (auto func : *callbacks) func();
    }

    static void printStats();

    static void clear() {
        for (auto& stat : statsData) stat = 0;
    }

private:
    /// @brief Static storage for statistics data
    static DataArray statsData;

    /// @brief Gathers functions that when called accumulate per thread statistics
    static CallbacksArray* callbacks;
};

/// @brief Collects statistics
#define STAT(statTest, countVar, registerer)          \
    static thread_local int64_t countVar;             \
    static StatRegisterer registerer([]() {           \
        StatRegisterer::data()[statTest] += countVar; \
        countVar = 0;                                 \
    })

/// @brief Called when thread begin work
void threadEntryPoint();

/// @brief Called when thread finish work
void threadExitPoint(const std::thread::id& threadId);

/// @brief When called per thread statistics is accumulated in the static storage and
/// each thread reports its running time
void reportThreadStats(const std::thread::id& threadId);

/// @brief Prints the collected statistics
void flushStatistics();

#endif  // !STATISTICS_H
