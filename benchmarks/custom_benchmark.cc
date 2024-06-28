// queue imports
#include "BasicSPSC.hh"
#include "BasicSPSCWithoutModulo.hh"
#include "SPSCWithRAPairs.hh"
#include "SPSCWithoutFS.hh"
#include "SPSCLocal.hh"
#include "fifo4.hh"

#include <benchmark/benchmark.h>

#include <iostream>
#include <format>
#include <thread>
#include <sched.h>  
#include <pthread.h> 
#include <stdio.h>
#include <sys/sysctl.h>
#include <utility>
#include <vector>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>
// MAC OS specific 
#include <Kernel/mach/thread_act.h>
#include <Kernel/mach/thread_policy.h>

template<typename T>
inline __attribute__((always_inline)) void doNotOptimize(T const& value) {
    asm volatile("" : : "r,m" (value) : "memory");
}

constexpr static int fifoSize = 131072; // 2048 * 8 * 8

template<typename T>
class Bench {
    T fifo;
    using queue_value_type = typename T::value_type;

    public:

    auto operator()(int64_t iterations) {
        
        auto th = std::thread([&] {
        // pop warmup
        for (auto i = queue_value_type{}; i < fifoSize; ++i) {
            queue_value_type val;
            while (not fifo.pop(val)) {
                    ;
            }
            doNotOptimize(val);

            if (val != i) {
                std::cout << "Errrrr::::   " << val << " " << i << std::endl;
                throw std::runtime_error("invalid value");
            }

        }

        // pop benchmark run
        for (auto i = queue_value_type{}; i < iterations; ++i) {
            queue_value_type val;
            while (not fifo.pop(val)) {
                    ;
            }
            doNotOptimize(val);

            if (val != i) {
                throw std::runtime_error("invalid value");
            }
        }
    });

    // push warmup
    for (auto i = queue_value_type{}; i < fifoSize; ++i) {
        while (auto again = not fifo.push(i)) {
            doNotOptimize(again);
        }
    }

    while (auto again = not fifo.empty()) {
        doNotOptimize(again);
    }

    // assert(fifo.empty());

    auto start = std::chrono::high_resolution_clock::now();
        // push warmup
        for (auto i = queue_value_type{}; i < iterations; ++i) {
            while (auto again = not fifo.push(i)) {
                doNotOptimize(again);
            }
        }

        while (auto again = not fifo.empty()) {
            doNotOptimize(again);
        }
        auto end = std::chrono::high_resolution_clock::now();

        th.join();
        return (iterations / std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count());
    }
};

int main() {
    constexpr auto iters = 10'000'000l;
    // constexpr auto iters = 100'000'000l;

    using value_type = std::int64_t;

    // auto opsPerSec = Bench<BasicSPSC<int_fast64_t, fifoSize>>{}(iters);
    // std::cout << std::fixed << std::showpoint;
    // std::cout << std::setprecision(10);
    // std::cout << std::setw(7) << std::left << "SPSCFifo: "
    //     << std::setw(10) << std::right << opsPerSec << " ops/s\n";

    auto ops = Bench<SPSCWithoutFS<int_fast64_t, fifoSize>>{}(iters);
    std::cout << std::fixed << std::showpoint;
    std::cout << std::setprecision(10);
    std::cout << std::setw(7) << std::left << "SPSCWithoutFS: "
        << std::setw(10) << std::right << ops << " ops/s\n"; 


    // auto ops = Bench<SPSCLocal<int_fast64_t, fifoSize>>{}(iters);
    // std::cout << std::fixed << std::showpoint;
    // std::cout << std::setprecision(10);
    // std::cout << std::setw(7) << std::left << "SPSCLocal: "
    //     << std::setw(10) << std::right << ops << " ops/s\n"; 

}