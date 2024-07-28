# ProducerConsumerQueues

## Overview
Project implements multiple single producer and single consumer lock-free fixed-size queues written in C++23 with focus on achieving low-latency and high-throughput between the producer and consumer thread. The project also includes benchmarks on performance for the below implementations. Implementation is tested as of now for fixed width types and fast integer types where lock free operations are supported inherently. 

The best implementation (spsc_local_cache) is faster than [*rigtorp::SPSCQueue*](https://github.com/rigtorp/SPSCQueue/blob/master/README.md?plain=1)

**Compatibility**: This project has been benchmarked on both ARM-based Darwin macOS M1 chipset and Intel-based x86-64 systems. It has been tested with both Clang-17 and GCC-13 compilers.

## Implementations
The project includes the following producer-consumer queue implementations:

1. Basic Single Producer Single Consumer (SPSC) Queue
2. Basic SPSC Queue without Modulo Operation
3. SPSC Queue with Read-Ahead Pairs
4. SPSC Queue without False Sharing
5. SPSC Queue with Local Cache ( Best version )
6. Rigtorp SPSC Queue

## Example
```cpp
#include "basic_spsc_queue.hpp"

int main() {
    basic_spsc_queue<int, 1024> queue;
    queue.push(42);
    int value;
    if (queue.pop(value)) {
        std::cout << "Popped value: " << value << std::endl;
    }
    return 0;
}
```

## Benchmarks
The following tables summarizes the benchmark results for each queue implementation under each system:

Performance improvements are significantly more noticeable on x86 systems, whereas similar gains are not as apparent on my ARM-based Mac-M1 Pro system.  The reasons for this discrepancy are not yet clear but could be related to differences in cache line architecture or other factors that require further investigation.

**Testing details**:
Producer and consumer run on two separate threads with each seperate thread pinned to a different cpu.
Below benchmarks are for `int_fast64_t` with size of the cache line taken as `64` for x86 and `128` for arm which is picked from `sysctl -a` command, though there are chances that mac supports multiple cache lines across different processors. Each queue is run for ten times and average throughput is recorded.

**Throughput** benchmark measure throughput between the two threads for queue

### x86

Pinning of threads to a cpu is supported in *x86* systems with the help of POSIX *pthread* library API's i.e., [*pthread_setaffinity_np*](https://www.man7.org/linux/man-pages/man3/pthread_setaffinity_np.3.html)


| Queue                             | Throughput (ops/s)  | 
| ----------------------------      | ------------------: |
| basic_spsc_queue                  |     10,582,807.68 |
| basic_spsc_without_modulo_queue   |              16,795,281.57 |
| spsc_ra_pairs |              76,502,970.44 |
| spsc_without_fs                  |     98,284,718.26 |
| spsc_local_cache   |              390,550,634.05 |
| rigtorp_spsc |   198,332,215.86|


### arm

Pinning of threads is not supported in darwin macOS, Though similar pinning can be tried as per [ref](https://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html). This does not yield any results as [*ThreadAffinityAPI*](https://developer.apple.com/library/archive/releasenotes/Performance/RN-AffinityAPI/) does not support this.


| Queue                             | Throughput (ops/s)  | 
| ----------------------------      | ------------------: |
| basic_spsc_queue                  |     13,604,841.48 |
| basic_spsc_without_modulo_queue   |              19,873,524.63 |
| spsc_ra_pairs |              15,650,168.62 |
| spsc_without_fs                  |     16,483,106.28 |
| spsc_local_cache   |              15,903,316.22 |
| rigtorp_spsc |   16,871,069.30|




## Getting Started
### Prerequisites
Ensure you have the following tools and libraries installed:

- C++ Compiler (e.g., g++, clang++)
- CMake
- Google Benchmark
- Google Test (optional, for unit tests)
- perf (optional, for benchmarking)
<!-- Boost (for lock-free data structures) -->

## Building the project
Once you have the repository cloned, please create two folders with the name **build** and **release**
To run in debug mode, please run *make* in *build* folder
To run in release (optimised) mode, run *make* in *release* folder

```zsh
mkdir [folder_name]
cd [folder_name]
cmake ..
make
```


## Run Benchmarks
To run the benchmarks, execute the following commands from the *release* directory:

```zsh
./basic_spsc_queue
./basic_spsc_without_modulo_queue
./spsc_ra_pairs
./spsc_without_fs
./spsc_local_cache
./rigtorp_spsc
```

Alternatively, you can run all benchmarks from the *scripts* folder:

```zsh
bash run_x86.sh                 ## Command to run on x86 based system
bash run calc_avg.sh            ## Command to run for average throughput
```

## Run Unit Tests
If you have included unit tests using Google Test, you can run them as follows from *build* folder:

```zsh
./unitTests
```

## Contributing
Contributions are welcome! Please submit a pull request or open an issue to discuss any changes or improvements.