add_executable(graphone_load graphone_load.cu)
set_target_properties(graphone_load PROPERTIES 
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
    CUDA_ARCHITECTURES ${CMAKE_CUDA_ARCHITECTURES}
    CUDA_SEPARABLE_COMPILATION ON
)