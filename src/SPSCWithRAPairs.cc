#include "custom_benchmark.hh"
#include "SPSCWithRAPairs.hh"

int main() {
    bench<SPSCWithRAPairs<int_fast64_t>>();
}