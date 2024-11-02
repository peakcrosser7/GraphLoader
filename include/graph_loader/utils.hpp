#pragma once

#include <string>

namespace graph_loader {

inline bool StrEndWith(const std::string& str, const std::string& suffix) {
    auto suf_size = suffix.size();
    if (suf_size == 0) {
        return true;
    }
    auto str_size = str.size();
    if (str_size == 0 || suf_size > str_size) {
        return false;
    }
    
    for (size_t i = 0, j = str_size - suf_size; i < suf_size; ++i, ++j) {
        if (suffix[i] != str[j]) {
            return false;
        }
    }
    return true;
}

inline bool StrStartWith(const std::string& str, const std::string& prefix) {
    auto pref_size = prefix.size();
    if (pref_size == 0) {
        return true;
    }
    auto str_size = str.size();
    if (str_size == 0 || pref_size > str_size) {
        return false;
    }
    for (size_t i = 0; i < pref_size; ++i) {
        if (prefix[i] != str[i]) {
            return false;
        }
    }
    return true;
}

inline std::string StrStrip(const std::string& str, const std::string& chars = " \t\n\r") {
    size_t start = str.find_first_not_of(chars);
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(chars);
    return str.substr(start, end - start + 1);
}


} // namespace graph_loader
