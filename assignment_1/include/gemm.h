#ifndef GEMM_H
#define GEMM_H

#include <vector>
#include <string>
#include <cstddef>

bool read_gemm_input(const std::string &path, int &M, int &K, int &N, Matrix &A, Matrix &B);


void gemm_simple(const Matrix &A, const Matrix &B, Matrix &C);


void gemm_blocking(const Matrix &A, const Matrix &B, Matrix &C, int block_size);

void print_matrix(const Matrix &C);

#endif 