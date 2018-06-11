file(GLOB DOT_H *.h)
file(GLOB DOT_CPP *.cpp)

add_library(VulkanRenderer STATIC ${DOT_H} ${DOT_CPP})
target_link_libraries(VulkanRenderer Framework ${Vulkan_LIBRARIES})