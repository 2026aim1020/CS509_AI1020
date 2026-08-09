#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
# Path to Assignment 1, CSR 
A1="${A1:-assignment_1}"
if [ ! -f "$A1/src/csr.cpp" ]; then
    echo "Error: can't find Assignment 1's src/csr.cpp at $A1"
    echo "Run with A1=/path/to/assignment1 ./build.sh if it's elsewhere."
    exit 1
fi

g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude -I"$A1/include" \
    "$A1/src/csr.cpp" assignment_2/src/bellman_ford.cpp assignment_2/src/bf_driver.cpp -o assignment_2/bf_driver

g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude -I"$A1/include" \
    assignment_2/src/floyd_warshall.cpp assignment_2/src/fw_driver.cpp -o assignment_2/fw_driver

g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude -I"$A1/include" \
    "$A1/src/csr.cpp" assignment_2/src/bellman_ford.cpp assignment_2/src/floyd_warshall.cpp assignment_2/src/cross_check_driver.cpp -o assignment_2/cross_check

# this is from outer folder run

g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude assignment_2/tools/gen_bf_tests.cpp -o assignment_2/tools/gen_bf_tests
g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude assignment_2/tools/gen_fw_tests.cpp -o assignment_2/tools/gen_fw_tests
g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude -I"$A1/include" \
    "$A1/src/csr.cpp" assignment_2/src/floyd_warshall.cpp assignment_2/tools/bf_to_fw_convert.cpp -o assignment_2/tools/bf_to_fw_convert

g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -pthread -Iassignment_1/include -Iassignment_2/include assignment_1/src/csr.cpp assignment_1/src/gemm.cpp assignment_2/src/bellman_ford.cpp assignment_2/src/floyd_warshall.cpp common_wrapper/wrapper.cpp -o wrapper.exe

echo "Build complete: ./bf_driver, ./fw_driver, ./cross_check, ./wrapper"