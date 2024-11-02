#include <string>
#include <iostream>

#include <igraph.h>

#include "igraph_loader.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " <mtx_path>\n";
        exit(-1);
    }
    std::string filename = argv[1];

    igraph_t g = IgraphLoader<int, int>::Load(filename, 
        graph_loader::LoaderOpts::Snap().set_is_directed(false));

    int num_v = igraph_vcount(&g);
    int num_e = igraph_ecount(&g);
    std::cout << "|V|:" << num_v << " |E|:" << num_e << std::endl;
    std::cout << "is_directed: " << igraph_is_directed(&g) << std::endl;
    
    igraph_destroy(&g);
    return 0;
}