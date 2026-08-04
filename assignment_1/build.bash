#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude src/gemm.cpp src/gemm_driver.cpp -o gemm_driver
g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -Iinclude src/csr.cpp src/csr_driver.cpp -o csr_driver
echo "Build complete: ./gemm_driver, ./csr_driver"
