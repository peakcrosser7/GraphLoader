#pragma once

#include <string>

namespace graph_loader {

struct Exception : std::exception {
    std::string report;

    Exception(std::string _message = "") { report = _message; }
    virtual const char *what() const noexcept { return report.c_str(); }
};


inline void throw_if_exception(bool is_exception, std::string message = "") {
    if (is_exception) {
        throw Exception(message);
    }
}

} // namespace graph_loader 