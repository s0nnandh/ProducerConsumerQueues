#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdlib> 
#include <numeric>
#include <vector>

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