cmake_minimum_required(VERSION 3.9)




project(KEngine2018)

find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan Not Found!")
endif (NOT Vulkan_FOUND)

include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/Framework)
include_directories(${PROJECT_SOURCE_DIR}/Game)
include_directories(${PROJECT_SOURCE_DIR}/Renderer)

add_subdirectory(Framework)
add_subdirectory(Game)
add_subdirectory(Renderer)
