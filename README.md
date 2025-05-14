# GraphLoader

GraphLoader is a header-only C++ library tailored for graph data loading.  
It supports formats like [Matrix Market](https://math.nist.gov/MatrixMarket/) and [SNAP](https://snap.stanford.edu/data/index.html), with flexible configurations for diverse graph processing scenarios.

## How to use

### Run the examples

The `examples` directory contains multiple sample codes to show how to use GraphLoader.  
You can compile some of the example programs with the following commands.  

```shell
mkdir build && cd build
cmake .. 
make
```

For detailed compilation and running instructions of each example, refer to the `README.md` in its corresponding directory.

### Integrate into your graph framework
Core API:

```cpp
template <typename vertex_t, typename edge_t, typename weight_t>
template <typename edge_load_func_t, 
          typename pre_load_func_t, 
          typename weight_parse_func_t>
static void GunrockLoader::Load(
    const std::string& filepath, 
    LoaderOpts& opts,
    const edge_load_func_t& edge_load_func, // bool FUNC(edge_t& eidx, vertex_t& src, vertex_t& dst, weight_t& val)
    const pre_load_func_t& pre_load_func,   // void FUNC() OR void FUNC(vertex_t num_v, edge_t num_e);
    const weight_parse_func_t& weight_parse_func    // weight_t FUNC(const char* str)
);
```

Implement the edge loading function `edge_load_func()` and the pre-loading function `pre_load_func()` (optional), and set the corresponding file configuration `opts`.
