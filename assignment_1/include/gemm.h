#ifndef GEMM_H
#define GEMM_H

#include <vector>
#include <string>
#include <cstddef>

struct Matrix
{
    int rows = 0;
    int cols = 0;
    std::vector<double> data; // size = rows * cols

    Matrix() = default;
    Matrix(int r, int c) : rows(r), cols(c), data(static_cast<size_t>(r) * c, 0.0) {}

    inline double &at(int i, int j) { return data[static_cast<size_t>(i) * cols + j]; }
    inline double at(int i, int j) const { return data[static_cast<size_t>(i) * cols + j]; }
};

bool read_gemm_input(const std::string &path, int &M, int &K, int &N, Matrix &A, Matrix &B);


void gemm_simple(const Matrix &A, const Matrix &B, Matrix &C);


void gemm_blocking(const Matrix &A, const Matrix &B, Matrix &C, int block_size);

void print_matrix(const Matrix &C);

#endif 