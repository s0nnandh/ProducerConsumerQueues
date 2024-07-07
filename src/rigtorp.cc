#include <type_traits>

#include "custom_benchmark.hh"
#include "rigtorp.hpp"

// explicit specialization for Rigtorp to evaluate to true
template <>
struct is_rigtorp<rigtorp::SPSCQueue<int_fast64_t>> : std::true_type {};

int main() {
    bench<rigtorp::SPSCQueue<int_fast64_t>>();
}