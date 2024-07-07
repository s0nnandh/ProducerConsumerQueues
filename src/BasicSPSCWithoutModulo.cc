#include "custom_benchmark.hh"
#include "BasicSPSCWithoutModulo.hh"

int main() {
    bench<BasicSPSCWithoutModulo<int_fast64_t>>();
}