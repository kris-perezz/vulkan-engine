
set(ENGINE_HEADER
  Log.h
  Application.h
  Window.h)

set(ENGINE_SOURCE
  Log.cpp
  Window.cpp
  Application.cpp)


add_library(Vulkan_Engine ${ENGINE_HEADER} ${ENGINE_SOURCE})

target_include_directories(
  Vulkan_Engine
  PUBLIC ${ENGINE_ROOT_PATH}
  PUBLIC ${ENGINE_ROOT_PATH}/src
  PUBLIC ${ENGINE_ROOT_PATH}/include)



target_link_libraries(
  Vulkan_Engine
  PUBLIC
  glm::glm
  spdlog::spdlog_header_only
)

add_executable(vulkan-engine
  main.cpp
)

target_link_libraries(vulkan-engine
  PRIVATE
    Vulkan_Engine
    Vulkan::Vulkan
    glfw
)