#include <benchmark/benchmark.h>

#include <iostream>
#include <thread>
#include <sched.h>  
#include <pthread.h> 
#include <stdio.h>
#include <sys/sysctl.h>
// MAC OS specific 
#include <Kernel/mach/thread_act.h>
#include <Kernel/mach/thread_policy.h>

// https://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html

#define SYSCTL_CORE_COUNT   "machdep.cpu.core_count"

typedef struct cpu_set {
  uint32_t    count;
} cpu_set_t;

static inline void
CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void
CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

static inline int
CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

int sched_getaffinity(pid_t pid, size_t cpu_size, cpu_set_t *cpu_set)
{
  int32_t core_count = 0;
  size_t  len = sizeof(core_count);
  int ret = sysctlbyname(SYSCTL_CORE_COUNT, &core_count, &len, 0, 0);
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
    if (::pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == -1) {
        std::perror("pthread_setaffinity_rp");
        std::exit(EXIT_FAILURE);
    }
}

static void BM_add(benchmark::State& state) {
    const int N = 1 << state.range(0);
    int res = 0;
    std::vector<int> v(N);
    std::generate(begin(v), end(v), []() {
      return rand() % 100;
    });

  for (auto _ : state) {
    for (int i = 0;i < N;++i) {
        benchmark::DoNotOptimize(res += v[i]);
    }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_add) -> DenseRange(10, 12) -> Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();