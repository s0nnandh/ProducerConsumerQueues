// queue imports
#include "BasicSPSC.hh"
#include "BasicSPSCWithoutModulo.hh"
#include "SPSCWithRAPairs.hh"
#include "SPSCWithoutFS.hh"
#include "SPSCLocal.hh"
#include <boost/lockfree/spsc_queue.hpp>

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
// MAC OS specific 
#include <Kernel/mach/thread_act.h>
#include <Kernel/mach/thread_policy.h>

// https://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html

#define SYSCTL_CORE_COUNT   "machdep.cpu.core_count"
#define CACHE_LINE_SIZE     "hw.cachelinesize"

typedef struct cpu_set {
  uint32_t    count;
} cpu_set_t;

static inline void
CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void
CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

static inline int
CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

int64_t cacheLineSize() {
    int64_t ret = 0;
    size_t size = sizeof(ret);
    
    if (sysctlbyname("hw.cachelinesize", &ret, &size, NULL, 0) == -1) {
        return -1;
    }
    
    return ret;
}

int sched_getaffinity(cpu_set_t *cpu_set)
{
  int32_t core_count = 0;
  size_t  len = sizeof(core_count);
  int ret = sysctlbyname(CACHE_LINE_SIZE, &core_count, &len, 0, 0);
  if (ret) {
    std::cout << "error while get core count" << "sysctlbyname returned " << ret << std::endl;
    return ret;
  }
  cpu_set->count = 0;
  std::cout << "Number of cores in system: " << core_count << std::endl;
  for (int i = 0; i < core_count; i++) {
    cpu_set->count |= (1 << i);
  }

  return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t cpu_size,
                           cpu_set_t *cpu_set)
{
  thread_port_t mach_thread;
  int core = 0;
  // TODO: can be improved  
  for (core = 0; core < 8 * cpu_size; core++) {
    if (CPU_ISSET(core, cpu_set)) break;
  }
  printf("binding to core %d\n", core);
  thread_affinity_policy_data_t policy = { core };
  mach_thread = pthread_mach_thread_np(thread);
  thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policy, 1);
  return 0;
}


static void pinThread(int cpu) {
    if (cpu < 0) {
        return;
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == -1) {
        std::perror("pthread_setaffinity_rp");
        std::exit(EXIT_FAILURE);
    }
}

constexpr static int fifoSize = 131072; // 2048 * 8 * 8

template<typename T>
static void BM_queue(benchmark::State& state) {

    constexpr long iterations = 10'000'000l;
    T fifo;
    using queue_value_type = typename T::value_type;

    const int cap = fifo.capacity();

   // jthread are missing part of mac OS hence reverting back to thread
   auto th = std::thread([&] {
        // pinThread(8);
        cpu_set cp;

        // pop warmup
        for (auto i = queue_value_type{}; i < cap; ++i) {
            queue_value_type val;
            while (not fifo.pop(val)) {
                    ;
            }
            benchmark::DoNotOptimize(val);

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
            benchmark::DoNotOptimize(val);

            if (val != i) {
                throw std::runtime_error("invalid value");
            }
        }
    });

    // pinThread(9);
    // cpu_set cp;
    // std::cout << cacheLineSize() << std::endl;

    // push warmup
    for (auto i = queue_value_type{}; i < cap; ++i) {
        while (auto again = not fifo.push(i)) {
            benchmark::DoNotOptimize(again);
        }
    }

    while (auto again = not fifo.empty()) {
        benchmark::DoNotOptimize(again);
    }

    // assert(fifo.empty());

    for (auto _ : state) {
            auto start = std::chrono::high_resolution_clock::now();
        // push warmup
        for (auto i = queue_value_type{}; i < iterations; ++i) {
            while (auto again = not fifo.push(i)) {
                benchmark::DoNotOptimize(again);
            }
        }

        while (auto again = not fifo.empty()) {
            benchmark::DoNotOptimize(again);
        }

        // assert(fifo.empty());

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
        end - start);

    state.SetIterationTime(elapsed_seconds.count());
    state.SetLabel(std::format("ops/sec: {}\n", (iterations) / (elapsed_seconds.count())));
    } 
    // state.counters["ops/sec"] = benchmark::Counter(double(iterations), benchmark::Counter::kIsRate);
    // state.PauseTiming();
    th.join();

}

using tt = std::int_fast64_t;

// manual timing
BENCHMARK_TEMPLATE(BM_queue, BasicSPSC<tt, fifoSize>) -> Unit(benchmark::kMicrosecond) -> UseManualTime();
// BENCHMARK_TEMPLATE(BM_queue, BasicSPSCWithoutModulo<tt, fifoSize>) -> Unit(benchmark::kMicrosecond) -> UseManualTime();
// BENCHMARK_TEMPLATE(BM_queue, SPSCWithRAPairs<tt, fifoSize>) -> Unit(benchmark::kMicrosecond) -> UseManualTime();
// BENCHMARK_TEMPLATE(BM_queue, SPSCWithoutFS<tt, fifoSize>) -> Unit(benchmark::kMicrosecond) -> UseManualTime();
// BENCHMARK_TEMPLATE(BM_queue, SPSCLocal<tt, fifoSize>) -> Unit(benchmark::kMicrosecond) -> UseManualTime();

// BENCHMARK_TEMPLATE(BM_queue, BasicSPSC<tt, fifoSize>) -> Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(BM_queue, BasicSPSCWithoutModulo<tt, fifoSize>) -> Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(BM_queue, SPSCWithRAPairs<tt, fifoSize>) -> Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(BM_queue, SPSCWithoutFS<tt, fifoSize>) -> Unit(benchmark::kMicrosecond);
// BENCHMARK_TEMPLATE(BM_queue, SPSCLocal<tt, fifoSize>) -> Unit(benchmark::kMicrosecond);


BENCHMARK_MAIN();