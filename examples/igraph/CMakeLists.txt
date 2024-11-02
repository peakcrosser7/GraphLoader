find_package(igraph REQUIRED)

add_executable(igraph_load igraph_load.cpp)
target_link_libraries(igraph_load PUBLIC igraph::igraph)