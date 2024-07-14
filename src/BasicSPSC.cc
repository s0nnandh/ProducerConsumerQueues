#include "custom_benchmark.hh"
#include "BasicSPSC.hh"

int main() {
    bench<BasicSPSC<int_fast64_t>>();
}