#!/bin/bash

# Print CPU information
lscpu

# Run performance stats on each executable in the ../release/ folder
perf stat ../release/basic_spsc_queue
perf stat ../release/basic_spsc_without_modulo_queue
perf stat ../release/spsc_ra_pairs
perf stat ../release/spsc_without_fs
perf stat ../release/spsc_local_cache
perf stat ../release/rigtorp_spsc
