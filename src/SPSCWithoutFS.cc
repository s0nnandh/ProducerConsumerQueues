#include "custom_benchmark.hh"
#include "SPSCWithoutFS.hh"

int main() {
    bench<SPSCWithoutFS<int_fast64_t>>();
}