#pragma once

#include <string>
#include <cassert>
#include <vector>
#include <algorithm>

// #define DEBUG_LOG
#include "graphpi/graph.h"
#include "graphpi/vertex_set.h"

#include "graph_loader/loader.hpp"

class GraphpiLoader {
public:
    using vertex_t = int;
    using edge_t = unsigned;
    using Loader = graph_loader::CoreLoader<vertex_t, edge_t, graph_loader::empty_t>;

    // you'better add COPY-CONSTRUCTOR function for Graph in GraphPi
    static Graph Load(const std::string& filepath, graph_loader::LoaderOpts opts, int oriented_type = 0) {
        using namespace graph_loader;
        assert(opts.undirected());

        Graph g;
        std::vector<vertex_t> degrees;
        std::vector<std::pair<vertex_t, vertex_t>> edges;

        auto pre_load_func = [&](vertex_t num_v, edge_t num_e) {
            g.v_cnt = num_v;
            g.e_cnt = num_e * 2;
            degrees.resize(num_v);
            edges.reserve(g.e_cnt);
        };

        auto edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst) -> bool {
            if (src == dst) {
                g.e_cnt -= 2;
                return false;
            }
            edges.emplace_back(src, dst);
            edges.emplace_back(dst, src);
            ++degrees[src];
            ++degrees[dst];
            return true;
        };

        Loader::Load(filepath, opts, edge_load_func, pre_load_func);

        assert(g.e_cnt == edges.size());

        PostLoad_(g, degrees, edges, oriented_type);

        return g;
    }

private:
    static void DoOriented_(Graph& g, 
                            std::vector<vertex_t>& degrees,
                            std::vector<std::pair<vertex_t, vertex_t>>& edges,
                            int oriented_type) {
        // oriented_type == 0 do nothing
        //               == 1 high degree first
        //               == 2 low degree first
        if (oriented_type == 0) {
            return;
        }

        std::vector<std::pair<vertex_t, vertex_t>> rank(g.v_cnt);
        std::vector<vertex_t> new_ids(g.v_cnt);

        for (vertex_t i = 0; i < g.v_cnt; ++i) {
            rank[i] = std::make_pair(i, degrees[i]);
        }

        if (oriented_type == 1) {
            std::sort(rank.begin(), rank.end(), [](const std::pair<vertex_t, vertex_t>& lhs, const std::pair<vertex_t, vertex_t>& rhs) {
                return lhs.second > rhs.second;
            });
        } else if (oriented_type == 2) {
            std::sort(rank.begin(), rank.end(), [](const std::pair<vertex_t, vertex_t>& lhs, const std::pair<vertex_t, vertex_t>& rhs) {
                return lhs.second < rhs.second;
            });               
        }

        for (vertex_t i = 0; i < g.v_cnt; ++i) {
            new_ids[rank[i].first] = i;
        }

        for (edge_t i = 0; i < g.e_cnt; ++i) {
            edges[i].first = new_ids[edges[i].first];
            edges[i].second = new_ids[edges[i].second];
        }     
    }

    static void PostLoad_(Graph& g, 
                          std::vector<vertex_t>& degrees,
                          std::vector<std::pair<vertex_t, vertex_t>>& edges, 
                          int oriented_type) {
        DoOriented_(g, degrees, edges, oriented_type);
        
        std::sort(degrees.begin(), degrees.end());
        VertexSet::max_intersection_size = std::max(VertexSet::max_intersection_size, degrees[g.v_cnt - 2]);
        g.max_degree = degrees.back();

        std::sort(edges.begin(), edges.end(), [](const std::pair<vertex_t, vertex_t>& lhs, const std::pair<vertex_t, vertex_t>& rhs) {
            return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
        });
        g.e_cnt = std::unique(edges.begin(), edges.end()) - edges.begin();

        g.edge = new vertex_t[g.e_cnt];
        g.vertex = new edge_t[g.v_cnt + 1];

        std::vector<bool> have_edge(g.v_cnt, false);
        vertex_t lst_v = -1;
        for(edge_t i = 0; i < g.e_cnt; ++i) {
            if(edges[i].first != lst_v) {
                have_edge[edges[i].first] = true;
                g.vertex[edges[i].first] = i;
            }
            lst_v = edges[i].first;
            g.edge[i] = edges[i].second;
        }
        g.vertex[g.v_cnt] = g.e_cnt;
        for(vertex_t i = g.v_cnt - 1; i >= 0; --i) {
            if(!have_edge[i]) {
                g.vertex[i] = g.vertex[i + 1];
            }
        }
    }
};