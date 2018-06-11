file(GLOB DOT_H *.h)
file(GLOB DOT_CPP *.cpp)


add_library(DX12Renderer STATIC ${DOT_H} ${DOT_CPP})
target_link_libraries(DX12Renderer Framework d3d12 dxgi d3dcompiler)