cmake_minimum_required(VERSION 4.1.1)

project(
  vulkan-engine 
  VERSION 1.0 
  DESCRIPTION "A Vulkan engine"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENGINE_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})


include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG f7485100cb16498f202f64d21b567c3788efa234
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.15.3
)
FetchContent_MakeAvailable(spdlog)

find_package(Vulkan REQUIRED)

find_package(glfw3 REQUIRED)

add_subdirectory(src)
