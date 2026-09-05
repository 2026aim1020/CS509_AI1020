#ifndef PAGERANK_H
#define PAGERANK_H

#include "../../assignment_1/include/csr.h"
#include <string>
#include <vector>

struct PageRankInput
{
    AdjacencyList graph; // directed, unweighted
    double damping = 0.85;
    double tolerance = 1e-4;
    int max_iterations = 100;
};

struct PageRankResult
{
    std::vector<double> rank;
    int iterations = 0;
    bool converged = false;
};

bool read_pagerank_input(const std::string &path, PageRankInput &in, std::string &err);


PageRankResult pagerank(const CSR &csr, double damping, double tolerance, int max_iterations);

#endif
