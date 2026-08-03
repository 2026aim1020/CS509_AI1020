#include "gemm.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

bool read_gemm_input(const std::string &path, int &M, int &K, int &N, Matrix &A, Matrix &B)
{
    std::ifstream in(path);
    if (!in.is_open())
    {
        return false;
    }

    if (!(in >> M >> K >> N))
    {
        return false;
    }
    if (M <= 0 || K <= 0 || N <= 0)
    {
        return false;
    }

    A = Matrix(M, K);
    for (int i = 0; i < M; ++i)
    {
        for (int j = 0; j < K; ++j)
        {
            if (!(in >> A.at(i, j)))
                return false;
        }
    }

    B = Matrix(K, N);
    for (int i = 0; i < K; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            if (!(in >> B.at(i, j)))
                return false;
        }
    }

    return true;
}

// Simple GEMM
void gemm_simple(const Matrix &A, const Matrix &B, Matrix &C)
{
    const int M = A.rows, K = A.cols, N = B.cols;
    C = Matrix(M, N);

    for (int i = 0; i < M; ++i)
    {
        for (int k = 0; k < K; ++k)
        {
            double a_ik = A.at(i, k);
            for (int j = 0; j < N; ++j)
            {
                C.at(i, j) += a_ik * B.at(k, j);
            }
        }
    }
}

// Tiled/blocked GEMM
void gemm_blocking(const Matrix &A, const Matrix &B, Matrix &C, int block_size)
{
    const int M = A.rows, K = A.cols, N = B.cols;
    C = Matrix(M, N);

    if (block_size <= 0)
        block_size = 32; // default

    for (int ii = 0; ii < M; ii += block_size)
    {
        int i_max = std::min(ii + block_size, M);
        for (int kk = 0; kk < K; kk += block_size)
        {
            int k_max = std::min(kk + block_size, K);
            for (int jj = 0; jj < N; jj += block_size)
            {
                int j_max = std::min(jj + block_size, N);

                for (int i = ii; i < i_max; ++i)
                {
                    for (int k = kk; k < k_max; ++k)
                    {
                        double a_ik = A.at(i, k);
                        for (int j = jj; j < j_max; ++j)
                        {
                            C.at(i, j) += a_ik * B.at(k, j);
                        }
                    }
                }
            }
        }
    }
}

static void print_number(std::ostream &out, double v)
{
    // Print as an integer when the value is (numerically) whole, otherwise
    // fall back to a fixed number of decimal places.
    double rounded = std::round(v);
    if (std::abs(v - rounded) < 1e-9)
    {
        out << static_cast<long long>(rounded);
    }
    else
    {
        out.precision(6);
        out << v;
    }
}

void print_matrix(const Matrix &C)
{
    std::ostringstream out;
    for (int i = 0; i < C.rows; ++i)
    {
        for (int j = 0; j < C.cols; ++j)
        {
            print_number(out, C.at(i, j));
            if (j + 1 < C.cols)
                out << ' ';
        }
        out << '\n';
    }
    std::cout << out.str();
}
