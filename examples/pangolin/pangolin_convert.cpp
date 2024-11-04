#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <filesystem>

#include "CLI11/CLI11.hpp"

#include "pangolin_converter.hpp"

using namespace graph_loader;
namespace fs = std::filesystem;

using vidType = int32_t;
using eidType = int64_t;

int main(int argc, char** argv) {
    LoaderOpts opts = LoaderOpts::WithHeader();
    std::string filepath;
    std::string output_dir;

    CLI::App app;
    app.add_option("-i, --input_graph", filepath, "input graph dataset file")->required();
    app.add_option("-o,--output_dir", output_dir, "output directory to save result");
    app.add_flag("-d,--is_directed", opts.is_directed, "directed graph dataset (defalut 'false')");
    app.add_flag("-r,--reorder", opts.do_reorder, "reorder vertex id in graph dataset to compress (defalut 'false')");
    app.add_flag("--rm_self", opts.rm_self_loop, "remove self loop edges in graph (defalut 'false')");
    CLI11_PARSE(app, argc, argv);

    if (output_dir.empty()) {
        output_dir = fs::path(filepath).parent_path().string() + "/";
    } else if (output_dir.back() != '/') {
        output_dir += "/";
    }

    if (!fs::exists(output_dir)) {
        if (!fs::create_directories(output_dir)) {
            std::cout << "[ERROR] output directory is not exist but create failed\n";
            return -1;
        }
    } else if (!fs::is_directory(output_dir)) {
        std::cout << "[ERROR] output path is not a directory\n";
        return -1;
    }
    

    PangolinMeta<vidType> meta = 
        PangolinConverter<vidType, eidType>::Load(filepath, opts);
    
    auto& adj_list = meta.adj_list;
    vidType n = meta.adj_list.size();

    std::vector<eidType> row_offsets(n + 1);
    std::vector<vidType> column_indices;

    row_offsets[0] = 0;
    for (int i = 0; i < n; ++i) {
        std::sort(adj_list[i].begin(), adj_list[i].end());
        for (const auto& neighbor : adj_list[i]) {
            column_indices.push_back(neighbor);
        }
        row_offsets[i + 1] = column_indices.size();
    }

    eidType m = column_indices.size();

    {
        std::ofstream meta_file(output_dir + "graph.meta.txt");
        meta_file << n << std::endl;
        meta_file << m << std::endl;
        meta_file << sizeof(vidType) << ' ' << sizeof(eidType) << ' ' << 1 << ' ' << 2 << std::endl;
        meta_file << meta.max_degree << std::endl;
        meta_file << 0 << std::endl << 1 << std::endl << 1 << std::endl;
    }
 
    {
        std::ofstream vertex_file(output_dir + "graph.vertex.bin", std::ios::binary);
        vertex_file.write(reinterpret_cast<const char*>(row_offsets.data()), (n + 1) * sizeof(eidType));
    }

    {
        std::ofstream edge_file(output_dir + "graph.edge.bin", std::ios::binary);
        edge_file.write(reinterpret_cast<const char*>(column_indices.data()), m * sizeof(vidType));
    }

    std::cout << "CONVERT: " << filepath << " OK\n";

    return 0;
}