#pragma once

#include <string>
#include <vector>
#include <cassert>

#include "graph_loader/loader.hpp"

template <typename vertex_t>
struct PangolinMeta {
    std::vector<std::vector<vertex_t>> adj_list;
    vertex_t max_degree = 0;
};

template <typename vertex_t, typename edge_t>
class PangolinConverter {
public:
    using Loader = graph_loader::CoreLoader<vertex_t, edge_t, graph_loader::empty_t>;

    static PangolinMeta<vertex_t> 
    Load(const std::string& filepath, graph_loader::LoaderOpts opts) {
        PangolinMeta<vertex_t> meta;

        auto pre_load_func = [&](vertex_t num_v, edge_t num_e) {
            meta.adj_list.resize(num_v);
        };

        auto edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst) -> bool {
            meta.adj_list[src].push_back(dst);
            meta.max_degree = std::max<vertex_t>(meta.max_degree, meta.adj_list[src].size());
            if (opts.undirected() && src != dst) {
                meta.adj_list[dst].push_back(src);
                meta.max_degree = std::max<vertex_t>(meta.max_degree, meta.adj_list[dst].size());
            }
            return true;
        };

        Loader::Load(filepath, opts, edge_load_func, pre_load_func);

        return meta;
    }
     
};