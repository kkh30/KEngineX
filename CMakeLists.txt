cmake_minimum_required(VERSION 3.9)




project(KEngine2018)

find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan Not Found!")
endif (NOT Vulkan_FOUND)

link_directories("C:\\VulkanSDK\\1.1.73.0\\Lib")
include_directories(${PROJECT_SOURCE_DIR}/Framework)
include_directories(${PROJECT_SOURCE_DIR}/Game)
include_directories(${PROJECT_SOURCE_DIR}/Renderer)

add_subdirectory(Framework)
add_subdirectory(Renderer)
add_subdirectory(Game)
