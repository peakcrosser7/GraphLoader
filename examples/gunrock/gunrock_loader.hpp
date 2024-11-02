#pragma once

#include <string>
#include <optional>

#include <gunrock/formats/coo.hxx>

#include "graph_loader/loader.hpp"

template <typename vertex_t, typename edge_t, typename weight_t>
class GunrockLoader {
public:
    using Loader = graph_loader::CoreLoader<vertex_t, edge_t, weight_t>;

    using memory_space_t = gunrock::memory::memory_space_t;
    using coo_t = gunrock::format::coo_t<memory_space_t::host, vertex_t, edge_t, weight_t>; 

    static auto Load(const std::string& filename, graph_loader::LoaderOpts opts) {
        std::optional<coo_t> coo;
        auto pre_load_func = [&](vertex_t num_v, edge_t num_e) {
            coo.emplace(num_v, num_v, num_e);
        };

        auto edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst, weight_t& val) -> bool {
            coo->row_indices[eidx] = src;
            coo->column_indices[eidx] = dst;
            coo->nonzero_values[eidx] = val;
            return true;
        };

        Loader::Load(filename, opts, edge_load_func, pre_load_func);
        PostLoad_(coo.value(), opts);

        return coo.value();
    }

private:
    static void PostLoad_(coo_t& coo, const graph_loader::LoaderOpts& opts) {
        if (!opts.is_directed) {  // duplicate off diagonal entries
            vertex_t off_diagonals = 0;
            for (vertex_t i = 0; i < coo.number_of_nonzeros; ++i) {
                if (coo.row_indices[i] != coo.column_indices[i])
                ++off_diagonals;
            }

            vertex_t _nonzeros =
                2 * off_diagonals + (coo.number_of_nonzeros - off_diagonals);

            gunrock::vector_t<vertex_t, memory_space_t::host> new_I(_nonzeros);
            gunrock::vector_t<vertex_t, memory_space_t::host> new_J(_nonzeros);
            gunrock::vector_t<weight_t, memory_space_t::host> new_V(_nonzeros);

            vertex_t* _I = new_I.data();
            vertex_t* _J = new_J.data();
            weight_t* _V = new_V.data();

            vertex_t ptr = 0;
            for (vertex_t i = 0; i < coo.number_of_nonzeros; ++i) {
                if (coo.row_indices[i] != coo.column_indices[i]) {
                _I[ptr] = coo.row_indices[i];
                _J[ptr] = coo.column_indices[i];
                _V[ptr] = coo.nonzero_values[i];
                ++ptr;
                _J[ptr] = coo.row_indices[i];
                _I[ptr] = coo.column_indices[i];
                _V[ptr] = coo.nonzero_values[i];
                ++ptr;
                } else {
                _I[ptr] = coo.row_indices[i];
                _J[ptr] = coo.column_indices[i];
                _V[ptr] = coo.nonzero_values[i];
                ++ptr;
                }
            }
            coo.row_indices = new_I;
            coo.column_indices = new_J;
            coo.nonzero_values = new_V;
            coo.number_of_nonzeros = _nonzeros;
        }  // end symmetric case
    }
};