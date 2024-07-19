#!/bin/bash

# Function to run an executable 10 times and calculate the average throughput
calculate_average_throughput() {
    local executable=$1
    local total_throughput=0

    for i in {1..10}; do
        # Run the executable and capture the output
        output=$("$executable")
        
        # Extract the second value from the output line (throughput) using awk
        throughput=$(echo "$output" | awk '{print $2}')
        
        # Add the throughput to the total
        total_throughput=$(echo "$total_throughput + $throughput" | bc)
    done

    # Calculate the average throughput
    average_throughput=$(echo "scale=2; $total_throughput / 10" | bc)
    echo "Average throughput for $executable: $average_throughput"
}

# Main script starts here
if [[ -z "$1" ]]; then
    echo "Usage: $0 <list_of_executables>"
    exit 1
fi

# Read the list of executables from the input file
executables=$(cat "$1")

# Run each executable and calculate the average throughput
for executable in $executables; do
    if [[ -x "$executable" ]]; then
        calculate_average_throughput "$executable"
    else
        echo "Executable $executable not found or is not executable"
    fi
done
