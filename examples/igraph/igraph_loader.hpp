#pragma once

#include <string>

#include <igraph.h>

#include "graph_loader/loader.hpp"


template <typename vertex_t, typename edge_t>
struct IgraphLoader;

template <>
struct IgraphLoader<int, int> {
    using vertex_t = int;
    using edge_t = int;
    using Loader = graph_loader::CoreLoader<vertex_t, edge_t, graph_loader::empty_t>;

    static igraph_t Load(const std::string& filename, graph_loader::LoaderOpts opts) {
        igraph_vector_int_t edges;

        auto pre_load_func = [&](vertex_t num_v, edge_t num_e) {
            igraph_vector_int_init(&edges, num_e * 2);
        };

        auto edge_load_func = [&](int eidx, int& src, int& dst) -> bool {
            VECTOR(edges)[eidx * 2] = src;
            VECTOR(edges)[eidx * 2 + 1] = dst;
            return true;
        };

        Loader::Load(filename, opts, edge_load_func, pre_load_func);

        igraph_t g;
        igraph_create(&g, &edges, 0, opts.is_directed);
        igraph_vector_int_destroy(&edges);
        return g;
    }
};