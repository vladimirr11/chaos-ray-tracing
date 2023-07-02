#include "Statistics.h"
#include "Timer.h"

/// @brief TLS timer used to measure the running time of each worker thread
static thread_local Timer threadRunTimeTimer;

StatRegisterer::DataArray StatRegisterer::statsData;
StatRegisterer::CallbacksArray* StatRegisterer::callbacks;

StatRegisterer::StatRegisterer(std::function<void()> f) {
    if (!callbacks)
        callbacks = new CallbacksArray;
    callbacks->push_back(f);
}

void StatRegisterer::printStats() {
    std::cout << "Ray-triangle intersection tests: " << statsData[NUM_TRIANGLE_ISECT_TESTS] << "\n"
              << "Actual ray-trinagle intersections: " << statsData[NUM_TRIANGLE_ISECTS] << "\n";
}

void threadEntryPoint() { threadRunTimeTimer.start(); }

void threadExitPoint(const std::thread::id& threadId) {
    std::cout << std::fixed << std::setprecision(2) << "Thread " << threadId << " render time ["
              << Timer::toMilliSec<float>(threadRunTimeTimer.getElapsedNanoSec()) << "ms]\n";
}

void reportThreadStats(const std::thread::id& threadId) {
    StatRegisterer::invokeCallbacks();
    threadExitPoint(threadId);
}

void flushStatistics() {
    StatRegisterer::printStats();
    StatRegisterer::clear();
}
