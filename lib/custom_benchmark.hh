#include <benchmark/benchmark.h>

#include <iostream>
#include <iomanip>
#include <thread>
#include <type_traits>
#include <vector>
// MAC OS specific 
#ifdef APPLE_H
#include <sys/sysctl.h>
#include <Kernel/mach/thread_act.h>
#include <Kernel/mach/thread_policy.h>
#endif

template<typename T>
inline __attribute__((always_inline)) void doNotOptimize(T const& value) {
    asm volatile("" : : "r,m" (value) : "memory");
}

// [SFINAE] default template for rigtorp to evaluate to false
template<typename T>
struct is_rigtorp : std::false_type {};

template<typename T>
class Bench {
    T fifo;
    using queue_value_type = typename T::value_type;
    // static constexpr bool is_rig = is_rigtorp<T>::value;

    public:

    auto operator()(int64_t iterations) {
        
        // Currently no support for jthread in clang 17
        auto th = std::thread([&] {
        // pop warmup
            for (auto i = queue_value_type{}; i < fifo.capacity(); ++i) {
                queue_value_type val;
                if constexpr (is_rigtorp<T>::value) {
                    while (auto again = not fifo.front()) {
                        doNotOptimize(again);
	                }
                    val = *fifo.front();
                    fifo.pop();
                } else {
                    while (auto again = not fifo.pop(val)) {
                        doNotOptimize(again);
                    }
                    doNotOptimize(val);
                }

                if (val != i) {
                    std::cout << "Errrrr::::   " << val << " " << i << std::endl;
                    throw std::runtime_error("invalid value");
                }

            }

            // pop benchmark run
            for (auto i = queue_value_type{}; i < iterations; ++i) {
                queue_value_type val;
                if constexpr (is_rigtorp<T>::value) {
                    while (auto again = not fifo.front()) {
                        doNotOptimize(again);
	                }
                    val = *fifo.front();
                    fifo.pop();
                } else {
                    while (auto again = not fifo.pop(val)) {
                        doNotOptimize(again);
                    }
                    doNotOptimize(val);
                }
                if (val != i) {
                    throw std::runtime_error("invalid value");
                }
            }
        });

        // push warmup
        for (auto i = queue_value_type{}; i < fifo.capacity(); ++i) {
            if constexpr (is_rigtorp<T>::value) {
                while (auto again = not fifo.try_push(i)) {
                    doNotOptimize(again);
                }
            } else {
                while (auto again = not fifo.push(i)) {
                    doNotOptimize(again);
                }
            }
        }

        while (auto again = not fifo.empty()) {
            doNotOptimize(again);
        }

    assert(fifo.empty());

    auto start = std::chrono::high_resolution_clock::now();
        // push test
        for (auto i = queue_value_type{}; i < iterations; ++i) {
            if constexpr (is_rigtorp<T>::value) {
                while (auto again = not fifo.try_push(i)) {
                    doNotOptimize(again);
                }
            } else {
                while (auto again = not fifo.push(i)) {
                    doNotOptimize(again);
                }
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


template<class T>
void bench() {   
    constexpr auto iters = 400'000'000l;
    // constexpr auto iters = 100'000'000l;

    auto opsPerSec = Bench<T>{}(iters);
    std::cout << std::fixed << std::showpoint;
    std::cout << std::setprecision(10);
    std::cout << std::setw(7) << std::left << "SPSCFifo: "
        << std::setw(10) << std::right << opsPerSec << " ops/s\n";
}