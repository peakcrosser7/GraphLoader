#pragma once

#include <algorithm>

#include "GraphOne/loader/edge_cache.h"

#include "graph_loader/loader.hpp"


template <typename vertex_t, typename edge_t, typename weight_t>
class GraphoneLoader {
public:
    using Loader = graph_loader::CoreLoader<vertex_t, edge_t, weight_t>;
    using edge_cache_t = graph_one::EdgeCache<graph_one::vstart_t::FROM_0_TO_0, weight_t, vertex_t, edge_t>;

    static edge_cache_t Load(const std::string& filepath, graph_loader::LoaderOpts opts,
                             bool keep_self_loop = false, bool keep_duplicate_edges = false) {
        using edge_uint_t = graph_one::EdgeUnit<weight_t, vertex_t>;

        edge_cache_t edge_cache;
        size_t init_cap = (1 << 12) / sizeof(edge_uint_t);
        edge_cache.reserve(init_cap);

        auto edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst, weight_t& val) -> bool {
            if (!keep_self_loop && src == dst) {
                return false;
            }

            edge_uint_t edge;
            edge.src = src;
            edge.dst = dst;
            edge.edata = val;
            edge_cache.push_back(edge);

            if (opts.undirected() && src != dst) {
                std::swap(edge.src, edge.dst);
                edge_cache.push_back(edge);
            }
            return true;
        };

        auto pre_load_func = []() {};

        Loader::Load(filepath, opts, edge_load_func, pre_load_func);

        if (!keep_duplicate_edges) {
            std::sort(edge_cache.begin(), edge_cache.end());
            edge_cache.erase(std::unique(edge_cache.begin(), edge_cache.end()), edge_cache.end());
        }

        return edge_cache;
    } 
};