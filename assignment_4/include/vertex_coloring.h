#ifndef VERTEX_COLORING_H
#define VERTEX_COLORING_H

#include "../../assignment_1/include/csr.h"
#include <string>
#include <vector>

struct ColoringResult
{
    std::vector<int> color; // color[v] for every vertex
    int colors_used = 0;
    bool valid = false; // true if no adjacent vertices share a color
};


bool validate_coloring_input(const AdjacencyList &g, std::string &err);


ColoringResult greedy_vertex_coloring(const CSR &csr);

bool check_coloring_valid(const CSR &csr, const std::vector<int> &color);

#endif
