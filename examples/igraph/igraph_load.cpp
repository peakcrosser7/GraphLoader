#include <iostream>
#include <string>

#include <igraph.h>

#include "igraph_loader.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <graph_file>\n";
        return -1;
    }

    std::string filepath = argv[1];

    igraph_t g = IgraphLoader<int, int>::Load(filepath, 
        graph_loader::OptsFactory::Snap().set_is_directed(false));
    // igraph_t g = IgraphLoader<int, int>::Load(filepath, 
    //     graph_loader::OptsFactory::MatrixMarket());

    int num_v = igraph_vcount(&g);
    int num_e = igraph_ecount(&g);
    std::cout << "|V|:" << num_v << " |E|:" << num_e << std::endl;
    std::cout << "is_directed: " << igraph_is_directed(&g) << std::endl;
    
    igraph_destroy(&g);
    return 0;
}