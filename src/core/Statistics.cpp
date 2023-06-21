#include "Statistics.h"
#include "Timer.h"

/// @brief TLS timer used to measure the running time of each worker thread
static thread_local Timer threadRunTimeTimer;

StatRegisterer::DataArray StatRegisterer::statsData;
StatRegisterer::CallbacksArray* StatRegisterer::callbacks;

StatRegisterer::StatRegisterer(std::function<void()> f) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (!callbacks)
        callbacks = new CallbacksArray;
    callbacks->push_back(f);
}

void StatRegisterer::printStats() {
    std::cout << "Ray-triangle intersection tests: " << statsData[NUM_TRIANGLE_ISECT_TESTS] << "\n"
              << "Actual ray-trinagle intersections: " << statsData[NUM_TRIANGLE_ISECTS]
              << std::endl;
}

void threadEntryPoint() { threadRunTimeTimer.start(); }

void threadExitPoint() {
    std::cout << std::fixed << std::setprecision(2) << "Thread render time ["
              << Timer::toMilliSec<float>(threadRunTimeTimer.getElapsedNanoSec()) << "ms]"
              << std::endl;
}

void reportThreadStats() {
    StatRegisterer::invokeCallbacks();
    threadExitPoint();
}

void flushStatistics() {
    StatRegisterer::printStats();
    StatRegisterer::clear();
}
