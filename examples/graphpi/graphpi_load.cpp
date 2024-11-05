#include <iostream>

#include "graphpi_loader.hpp"

using namespace graph_loader;

int main(int argc,char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <graph_file>\n";
        return -1;
    }

    std::string filepath = argv[1];

    Graph g = GraphpiLoader::Load(filepath, LoaderOpts::WithHeader().set_is_directed(false).set_do_reoder(true));

    auto num_v = g.v_cnt;
    auto num_e = g.e_cnt;
    std::cout << "|V|:" << num_v << " |E|:" << num_e << std::endl;

    return 0;    
}