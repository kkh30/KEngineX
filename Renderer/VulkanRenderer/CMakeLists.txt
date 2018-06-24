file(GLOB DOT_H *.h)
file(GLOB DOT_CPP *.cpp)



add_subdirectory(Anvil)

include_directories(${Anvil_SOURCE_DIR}/include)
target_include_directories(Anvil PUBLIC ${Anvil_SOURCE_DIR}/include)

add_library(VulkanRenderer STATIC ${DOT_H} ${DOT_CPP})

target_link_libraries(VulkanRenderer Framework Anvil)
