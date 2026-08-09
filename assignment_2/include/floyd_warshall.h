#ifndef FLOYD_WARSHALL_H
#define FLOYD_WARSHALL_H

#include <vector>
#include <string>

// Dense V x V distance matrix.
struct DenseMatrix
{
    int V = 0;
    std::vector<double> data; // row-major, size V*V

    DenseMatrix() = default;
    explicit DenseMatrix(int v) : V(v), data(static_cast<size_t>(v) * v, 0.0) {}

    inline double &at(int i, int j) { return data[static_cast<size_t>(i) * V + j]; }
    inline double at(int i, int j) const { return data[static_cast<size_t>(i) * V + j]; }
};

bool read_fw_input(const std::string &path, DenseMatrix &M);

struct FloydWarshallResult
{
    bool negative_cycle = false;
    DenseMatrix distance; // valid only if negative_cycle == false
};


FloydWarshallResult floyd_warshall(const DenseMatrix &M);


// Bellman-Ford and Floyd-Warshall can be run on the exact same graph.
class CSR; 
DenseMatrix csr_to_dense_matrix(const CSR &csr);

void print_fw_matrix(const DenseMatrix &M);

#endif 
