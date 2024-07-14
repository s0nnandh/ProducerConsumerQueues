#include "custom_benchmark.hh"
#include "SPSCLocal.hh"

int main() {
    bench<SPSCLocal<int_fast64_t>>();
}