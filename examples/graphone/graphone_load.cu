#include <iostream>

#include "GraphOne/graph/builder.h"

#include "graphone_loader.hpp"

using namespace graph_loader;
using namespace graph_one;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <graph_file>\n";
        return -1;
    }

    std::string filepath = argv[1];

    auto cache = GraphoneLoader<vid_t, eid_t, float>::Load(filepath, LoaderOpts::MatrixMarket());
    auto g = graph::build<arch_t::cpu, graph_view_t::csr | graph_view_t::normal>(cache);

    auto num_v = g.num_vertices();
    auto num_e = g.num_edges();
    std::cout << "|V|:" << num_v << " |E|:" << num_e << std::endl;

    return 0;    
}