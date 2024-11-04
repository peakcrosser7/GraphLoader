#pragma once

#include <string>
#include <functional>

#include "mm_typecode.hpp"

namespace graph_loader {

enum class BasedIndex {
    BASED_0_TO_0,
    BASED_1_TO_0
};

struct LoaderOpts {
    std::string file_ext = "";
    std::string comment_prefix = "";
    std::string line_sep = " ";
    std::string line_strips = " \t\n\r";
    BasedIndex based_index = BasedIndex::BASED_0_TO_0;
    int header_cnt = 2;
    bool is_directed = false;
    bool rm_self_loop = false;
    bool do_reorder = false;

    std::function<void(std::string&, LoaderOpts&)> load_banner_func;

    LoaderOpts& set_is_directed(bool directed) {
        is_directed = directed;
        return *this;
    }

    LoaderOpts& set_do_reoder(bool reorder) {
        do_reorder = reorder;
        return *this;
    }

    static LoaderOpts MatrixMarket() {
        LoaderOpts opts;
        opts.comment_prefix = "%";
        opts.file_ext = ".mtx";
        opts.line_sep = " ";
        opts.based_index = BasedIndex::BASED_1_TO_0;
        opts.header_cnt = 3;

        opts.load_banner_func = [&](std::string& line, LoaderOpts& opts_) {
            MMTypecode code = MMTypecode::FromString(line);    
            throw_if_exception(code.is_dense(), "File is not a sparse matrix");

            opts_.is_directed = !code.is_symmetric();
        };

        return opts;
    }

    static LoaderOpts Snap() {
        LoaderOpts opts;
        opts.file_ext = ".txt";
        opts.comment_prefix = "#";
        opts.line_sep = "\t";
        opts.header_cnt = 0;
        opts.do_reorder = true;

        return opts;
    }

    static LoaderOpts WithHeader() {
        LoaderOpts opts;
        opts.line_sep = " ";
        opts.header_cnt = 2;

        return opts;
    }
};


} // namespace graph_loader
