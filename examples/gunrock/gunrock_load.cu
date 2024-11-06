#include <iostream>
#include <string>

#include <gunrock/graph/graph.hxx>

#include "gunrock_loader.hpp"

using namespace graph_loader;
using namespace gunrock;
using namespace gunrock::memory;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <graph_file>\n";
        return -1;
    }

    std::string filepath = argv[1];

    using vertex_t = int;
    using edge_t = int;
    using weight_t = float;
    using csr_t = gunrock::format::csr_t<memory_space_t::device, vertex_t, edge_t, weight_t>;

    auto coo = GunrockLoader<vertex_t, edge_t, weight_t>::Load(filepath, OptsFactory::MatrixMarket());

    csr_t csr;
    csr.from_coo(coo);
    auto G = graph::build::from_csr<memory_space_t::device, gunrock::graph::view_t::csr>(
        csr.number_of_rows,               // rows
        csr.number_of_columns,            // columns
        csr.number_of_nonzeros,           // nonzeros
        csr.row_offsets.data().get(),     // row_offsets
        csr.column_indices.data().get(),  // column_indices
        csr.nonzero_values.data().get()   // values
    );  // supports row_indices and column_offsets (default = nullptr)


    vertex_t num_v = G.get_number_of_vertices();
    edge_t num_e = G.get_number_of_edges();
    std::cout << "|V|:" << num_v << " |E|:" << num_e << std::endl;
    std::cout << "is_directed: " << G.is_directed() << std::endl;

    return 0;
}