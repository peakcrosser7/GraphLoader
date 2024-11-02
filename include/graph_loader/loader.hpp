#pragma once

#include <string>
#include <fstream>

#include "exception.hpp"
#include "opts.hpp"
#include "helper.hpp"
#include "utils.hpp"

namespace graph_loader {

template <typename vertex_t, typename edge_t, typename value_t>
class CoreLoader { 
public:
    template <typename edge_load_func_t, 
              typename pre_load_func_t = decltype(dummy_func), 
              typename weight_parse_func_t = decltype(general_weight_parse<value_t>)>
    static void Load(
        const std::string& filepath, 
        LoaderOpts& opts,
        const edge_load_func_t& edge_load_func, // bool FUNC(edge_t& eidx, vertex_t& src, vertex_t& dst, value_t& val)
        const pre_load_func_t& pre_load_func = dummy_func,  // void FUNC() OR void FUNC(vertex_t num_v, edge_t num_e);
        const weight_parse_func_t& weight_parse_func = general_weight_parse<value_t>    // value_t FUNC(const char* str)
    ) {
        if (opts.header_cnt == 0) {
            LoadWithoutHeader(filepath, opts, edge_load_func, pre_load_func, weight_parse_func);
        } else {
            LoadWithHeader(filepath, opts, edge_load_func, pre_load_func, weight_parse_func);
        }
    }

    template <typename edge_load_func_t, 
              typename pre_load_func_t = decltype(dummy_func), 
              typename weight_parse_func_t = decltype(general_weight_parse<value_t>)>
    static void LoadWithoutHeader(
        const std::string& filepath, 
        LoaderOpts& opts,
        const edge_load_func_t& edge_load_func, // bool FUNC(edge_t& eidx, vertex_t& src, vertex_t& dst, value_t& val)
        const pre_load_func_t& pre_load_func = dummy_func,   // void FUNC() OR void FUNC(vertex_t num_v, edge_t num_e)
        const weight_parse_func_t& weight_parse_func = general_weight_parse<value_t>    // value_t FUNC(const char* str)
    ) {
        std::unordered_map<vertex_t, vertex_t> reordered_map;

        auto pre_load_wrapper = LoadHeaderSimulating_(filepath, opts, pre_load_func, reordered_map);

        if (opts.do_reorder) {
            auto edge_load_wrapper = MakeReorderedEdgeLoader_(opts.based_index, reordered_map, edge_load_func);
            LoadWithoutHeader_(filepath, opts, edge_load_wrapper, pre_load_wrapper, weight_parse_func);
        } else if (opts.based_index == BasedIndex::BASED_1_TO_0) {
            auto edge_load_wrapper = MakeBased1to0EdgeLoader_(edge_load_func);
            LoadWithoutHeader_(filepath, opts, edge_load_wrapper, pre_load_wrapper, weight_parse_func);
        } else {
            LoadWithoutHeader_(filepath, opts, edge_load_func, pre_load_wrapper, weight_parse_func);
        }
    }

    template <typename edge_load_func_t, 
              typename pre_load_func_t = decltype(dummy_func), 
              typename weight_parse_func_t = decltype(general_weight_parse<value_t>)>
    static void LoadWithHeader(
        const std::string& filepath, 
        LoaderOpts& opts,
        const edge_load_func_t& edge_load_func, // bool FUNC(edge_t& eidx, vertex_t& src, vertex_t& dst, value_t& val)
        const pre_load_func_t& pre_load_func = dummy_func,   // void FUNC() OR void FUNC(vertex_t num_v, edge_t num_e)
        const weight_parse_func_t& weight_parse_func = general_weight_parse<value_t>    // value_t FUNC(const char* str)
    ) {
        // std::cout << "debug 1\n";        
        if (opts.do_reorder) {
            std::unordered_map<vertex_t, vertex_t> reordered_map;
            auto edge_load_wrapper = MakeReorderedEdgeLoader_(opts.based_index, reordered_map, edge_load_func);
            LoadWithHeader_(filepath, opts, edge_load_wrapper, pre_load_func, weight_parse_func);
        } if (opts.based_index == BasedIndex::BASED_1_TO_0) {
            auto edge_load_wrapper = MakeBased1to0EdgeLoader_(edge_load_func);
            LoadWithHeader_(filepath, opts, edge_load_wrapper, pre_load_func, weight_parse_func);
        } else {
            LoadWithHeader_(filepath, opts, edge_load_func, pre_load_func, weight_parse_func);
        }        
    }

private:
    template <typename edge_load_func_t>
    static auto MakeReorderedEdgeLoader_(
        BasedIndex based_index,
        std::unordered_map<vertex_t, vertex_t>& reordered_map,
        const edge_load_func_t& edge_load_func    // bool FUNC(edge_t& eidx, vertex_t& src, vertex_t& dst, value_t& val)
    ) {
        // std::unordered_map<vertex_t, vertex_t> reordered_map;

        vertex_t base = (based_index == BasedIndex::BASED_0_TO_0 || based_index == BasedIndex::BASED_1_TO_0) ? 0 : 1;
        auto reorder_vid = [&reordered_map, base](vertex_t vid) -> vertex_t {
            auto [it,_] = reordered_map.insert({vid, vertex_t(reordered_map.size() + base)});
            return it->second;
        };

        if constexpr (std::is_same_v<value_t, empty_t>) {
            return [&edge_load_func, reorder_vid](edge_t eidx, vertex_t& src, vertex_t& dst) -> bool {
                src = reorder_vid(src);
                dst = reorder_vid(dst);
                // std::cout << "debug: makereorder\n";
                return edge_load_func(eidx, src, dst);
            };            
        } else {
            return [&edge_load_func, reorder_vid](edge_t eidx, vertex_t& src, vertex_t& dst, value_t& val) -> bool {
                src = reorder_vid(src);
                dst = reorder_vid(dst);
                // std::cout << "debug: makereorder\n";
                return edge_load_func(eidx, src, dst, val);
            };
        }
    }

    template <typename edge_load_func_t>
    static auto MakeBased1to0EdgeLoader_(const edge_load_func_t& edge_load_func) {
        if constexpr (std::is_same_v<value_t, empty_t>) {
            return [&](edge_t eidx, vertex_t& src, vertex_t& dst) -> bool {
                throw_if_exception(src == 0, "file is one-indexed but got 0");
                throw_if_exception(dst == 0, "file is one-indexed but got 0");
                return edge_load_func(eidx, --src, --dst);
            };
        } else {
            return [&](edge_t eidx, vertex_t& src, vertex_t& dst, value_t& val) -> bool {
                throw_if_exception(src == 0, "file is one-indexed but got 0");
                throw_if_exception(dst == 0, "file is one-indexed but got 0");
                return edge_load_func(eidx, --src, --dst, val);
            };
        }
    }

    static bool LoadLine_(
        std::ifstream& fin, 
        std::string& line, 
        const LoaderOpts& opts,
        bool do_check = true
    ) {
        while(fin.good() && !fin.eof()) {
            std::getline(fin, line);
            if (do_check) {
                line = StrStrip(line, opts.line_strips);
                if (line.empty() || StrStartWith(line, opts.comment_prefix)) {
                    continue;
                }
            }
            return true;
        }
        return false;
    }

    static std::ifstream LoadPrepare_(const std::string& filepath, LoaderOpts& opts) {
        throw_if_exception(!StrEndWith(filepath, opts.file_ext),
                           "File should have \"" + opts.file_ext + "\" file extension.");
        
        std::ifstream fin(filepath);
        throw_if_exception(fin.fail(), "Cannot open file: " + filepath);

        if (opts.load_banner_func) {
            std::string line;
            bool ok = LoadLine_(fin, line, opts, false);
            throw_if_exception(!ok, "Load file when calling pre_load_func");
            opts.load_banner_func(line, opts);
        }
        return fin;
    }

    template <typename pre_load_func_t>
    static auto LoadHeaderSimulating_(
        const std::string& filepath, 
        LoaderOpts& opts,
        const pre_load_func_t& pre_load_func,  // void FUNC(vertex_t num_v, edge_t num_e)
        std::unordered_map<vertex_t, vertex_t>& reordered_map,
        std::enable_if_t<std::is_invocable_r_v<void, pre_load_func_t, vertex_t, edge_t>>* = nullptr
    ) {
        vertex_t num_v;
        edge_t num_edges = 0;

        if constexpr (std::is_same_v<value_t, empty_t>) {
            if (opts.do_reorder) {
                auto get_ve_edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst) {
                        // std::cout <<  "num_edges" << num_edges << std::endl;
                        ++num_edges;
                        return true;
                };
                auto get_ve_edge_load_wrapper = MakeReorderedEdgeLoader_(opts.based_index, reordered_map, get_ve_edge_load_func);
                LoadWithoutHeader_(filepath, opts, get_ve_edge_load_wrapper);

                num_v = reordered_map.size();
                // std::cout << "debug: " << num_v << std::endl;
            } else {
                vertex_t max_id = -1;
                auto get_ve_edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst) {
                    max_id = std::max(max_id, src);
                    max_id = std::max(max_id, dst);
                    ++num_edges;
                    return true;
                };
                LoadWithoutHeader_(filepath, opts, get_ve_edge_load_func);
                
                num_v = max_id + (opts.based_index == BasedIndex::BASED_0_TO_0);
            }
        } else {
            if (opts.do_reorder) {
                auto get_ve_edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst, value_t& val) {
                        // std::cout <<  "num_edges" << num_edges << std::endl;
                        ++num_edges;
                        return true;
                };
                auto get_ve_edge_load_wrapper = MakeReorderedEdgeLoader_(opts.based_index, reordered_map, get_ve_edge_load_func);
                LoadWithoutHeader_(filepath, opts, get_ve_edge_load_wrapper);

                num_v = reordered_map.size();
                // std::cout << "debug: " << num_v << std::endl;
            } else {
                vertex_t max_id = -1;
                auto get_ve_edge_load_func = [&](edge_t eidx, vertex_t& src, vertex_t& dst, value_t& val) {
                    max_id = std::max(max_id, src);
                    max_id = std::max(max_id, dst);
                    ++num_edges;
                    return true;
                };
                LoadWithoutHeader_(filepath, opts, get_ve_edge_load_func);
                
                num_v = max_id + (opts.based_index == BasedIndex::BASED_0_TO_0);
            }
        }

        return [num_v, num_edges, &pre_load_func]() {
            pre_load_func(num_v, num_edges);
        };
    }

    template <typename pre_load_func_t>
    static auto LoadHeaderSimulating_(
        const std::string& filepath, 
        LoaderOpts& opts,
        const pre_load_func_t& pre_load_func,  // void FUNC()
        std::unordered_map<vertex_t, vertex_t>& reordered_map,
        std::enable_if_t<std::is_invocable_r_v<void, pre_load_func_t, void>>* = nullptr
    ) {
        return pre_load_func;
    }
    
    static void ParseHeader2_(
        std::string& line, 
        const std::string& sep, 
        vertex_t& num_v, 
        edge_t& num_e
    ) {
        char* pSave  = nullptr;
        char* pToken = nullptr;

        pToken = strtok_r(line.data(), sep.c_str(), &pSave);
        throw_if_exception(pToken == nullptr, "fail to load num_v when calling ParseHeader2_");
        auto num_v_ = std::strtoul(pToken, nullptr, 10);
        throw_if_exception(num_v_ >= std::numeric_limits<vertex_t>::max(),
                        "vertex_t overflow when calling ParseHeader2_");

        pToken = strtok_r(nullptr, sep.c_str(), &pSave);
        throw_if_exception(pToken == nullptr, "fail to~ load num_e when calling ParseHeader2_");
        auto num_e_ = std::strtoul(pToken, nullptr, 10);
        throw_if_exception(num_e_ >= std::numeric_limits<edge_t>::max(), "edge_t overflow when calling ParseHeader2_");

        num_v = static_cast<vertex_t>(num_v_);
        num_e = static_cast<edge_t>(num_e_);
    }

    static void ParseHeader3_(
        std::string& line, 
        const std::string& sep, 
        vertex_t& num_v, 
        edge_t& num_e
    ) {
        char* pSave  = nullptr;
        char* pToken = nullptr;

        pToken = strtok_r(line.data(), sep.c_str(), &pSave);
        throw_if_exception(pToken == nullptr, "fail to load num_v when calling ParseHeader3_");
        auto num_rows = std::strtoul(pToken, nullptr, 10);

        pToken = strtok_r(nullptr, sep.c_str(), &pSave);
        throw_if_exception(pToken == nullptr, "fail to load num_cols when calling ParseHeader3_");
        auto num_cols = std::strtoul(pToken, nullptr, 10);

        throw_if_exception(num_rows >= std::numeric_limits<vertex_t>::max() ||
            num_cols >= std::numeric_limits<vertex_t>::max(),
            "vertex_t overflow when calling ParseHeader3_");
    
        throw_if_exception(num_rows != num_cols, "num_rows != num_cols i.e. the file is NOT a graph when calling ParseHeader3_."); 

        pToken = strtok_r(nullptr, sep.c_str(), &pSave);
        throw_if_exception(pToken == nullptr, "fail to load num_e when calling ParseHeader3_");
        auto nnz = std::strtoul(pToken, nullptr, 10);
        throw_if_exception(nnz >= std::numeric_limits<edge_t>::max(), "edge_t overflow when calling ParseHeader3_");

        num_v = static_cast<vertex_t>(num_rows);
        num_e = static_cast<edge_t>(nnz);    
    }

    template <typename pre_load_func_t>
    static void LoadHeader_(
        std::ifstream& fin, 
        const LoaderOpts& opts, 
        const pre_load_func_t& pre_load_func,   // void FUNC(vertex_t num_v, edge_t num_e)
        std::enable_if_t<std::is_invocable_r_v<void, pre_load_func_t, vertex_t, edge_t>>* = nullptr
    ) {
        std::string line;
        bool ok = LoadLine_(fin, line, opts);
        throw_if_exception(!ok, "LoadLine_ failed when calling LoadHeader_");

        vertex_t num_v;
        edge_t num_e;
        // std::cout << "debug-line:" << line << std::endl;
        if (opts.header_cnt == 2) {
            ParseHeader2_(line, opts.line_sep, num_v, num_e);
        } else if (opts.header_cnt == 3) {
            ParseHeader3_(line, opts.line_sep, num_v, num_e);
        } else {
            throw_if_exception(true, "unsupport header cnt: " + std::to_string(opts.header_cnt));
        }
        pre_load_func(num_v, num_e);
    }

    template <typename pre_load_func_t>
    static void LoadHeader_(
        std::ifstream& fin, 
        const LoaderOpts& opts, 
        const pre_load_func_t& pre_load_func,   // void FUNC()
        std::enable_if_t<std::is_invocable_r_v<void, pre_load_func_t, void>>* = nullptr
    ) {

        std::string line;
        bool ok = LoadLine_(fin, line, opts);
        throw_if_exception(!ok, "LoadLine_ failed when calling LoadHeader_");

        pre_load_func();
    }    

    template <typename edge_load_func_t, typename weight_parse_func_t>
    static void LoadEdges_(
        std::ifstream& fin, 
        const LoaderOpts& opts, 
        const edge_load_func_t& edge_load_func,
        const weight_parse_func_t& weight_parse_func
    ) {
        std::string line;
        char* pSave  = nullptr;
        char* pToken = nullptr;

        for (edge_t eidx = 0; true;) {
            if (!LoadLine_(fin, line, opts)) {
                break;
            }

            pToken = strtok_r(line.data(), opts.line_sep.c_str(), &pSave);
            if (nullptr == pToken) {
                // LOG_WARNING("can not extract source from (", pLog, ")");
                continue;
            }
            vertex_t src = vertex_t(std::strtoul(pToken, nullptr, 10));

            pToken = strtok_r(nullptr, opts.line_sep.c_str(), &pSave);
            if (nullptr == pToken) {
                // LOG_WARNING("can not extract destination from (", pLog, ")");
                continue;
            }
            vertex_t dst = vertex_t(std::strtoul(pToken, nullptr, 10));
            if (opts.rm_self_loop && src == dst) {
                continue;
            }

            // std::cout << "debug: " << src << " " << dst << std::endl;
            if constexpr (std::is_same_v<value_t, empty_t>) {
                if (edge_load_func(eidx, src, dst)) {
                    ++eidx;
                }                
            } else {
                pToken = strtok_r(nullptr, opts.line_sep.c_str(), &pSave);
                value_t val = weight_parse_func(pToken);
                if (edge_load_func(eidx, src, dst, val)) {
                    ++eidx;
                }
            }

            // std::cout << "debug load end\n";
        }
    }

    template <typename edge_load_func_t, 
              typename pre_load_func_t = decltype(dummy_func), 
              typename weight_parse_func_t = decltype(general_weight_parse<value_t>)>
    static void LoadWithoutHeader_(
        const std::string& filepath, 
        LoaderOpts& opts,
        const edge_load_func_t& edge_load_func,     // void FUNC(edge_t& eidx, vertex_t& src, vertex_t& dst, value_t& val)
        const pre_load_func_t& pre_load_func = dummy_func,  // void FUNC()
        const weight_parse_func_t& weight_parse_func = general_weight_parse<value_t>      // value_t FUNC(const char* str)
    ) {
        std::ifstream fin = LoadPrepare_(filepath, opts);

        // std::cout << "debug 0\n";
        pre_load_func();

        LoadEdges_(fin, opts, edge_load_func, weight_parse_func);
    }

    template <typename edge_load_func_t, 
              typename pre_load_func_t, 
              typename weight_parse_func_t = decltype(general_weight_parse<value_t>)>
    static void LoadWithHeader_(const std::string& filepath, 
                LoaderOpts& opts,
                const edge_load_func_t& edge_load_func,     // void FUNC(edge_t& eidx, vertex_t& src, vertex_t& dst, value_t& val)
                const pre_load_func_t& pre_load_func,   // void FUNC() OR void FUNC(vertex_t num_v, edge_t num_e)
                const weight_parse_func_t& weight_parse_func    // value_t FUNC(const char* str)
    ) {
        std::ifstream fin = LoadPrepare_(filepath, opts);

        // std::cout << "debug 0\n";
        LoadHeader_(fin, opts, pre_load_func);
        
        // std::cout << "debug 1\n";        
        LoadEdges_(fin, opts, edge_load_func, weight_parse_func);     
    }
};

} // namespace graph_loader



