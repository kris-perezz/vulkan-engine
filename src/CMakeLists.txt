
set(ENGINE_HEADER
  Log.h
  Renderer.h
  RenderSystem.h
  Application.h
  Window.h
  Pipeline.h
  EngineDevice.h
  SwapChain.h
  Model.h
  GameObject.h)

set(ENGINE_SOURCE
  Log.cpp
  Window.cpp
  Renderer.cpp
  RenderSystem.cpp
  Application.cpp
  Pipeline.cpp
  EngineDevice.cpp
  SwapChain.cpp
  Model.cpp)


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

set(PRE_SCRIPT "${CMAKE_SOURCE_DIR}/compileShader.sh")

add_custom_target(prebuild_always ALL
  COMMAND ${CMAKE_COMMAND} -E env bash "${PRE_SCRIPT}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_dependencies(vulkan-engine prebuild_always)

target_link_libraries(vulkan-engine
  PRIVATE
    Vulkan_Engine
    Vulkan::Vulkan
    glfw
)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)


add_custom_command(TARGET vulkan-engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
          $<TARGET_FILE_DIR:vulkan-engine>/shaders
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${SHADER_DIR}
          $<TARGET_FILE_DIR:vulkan-engine>/shaders
)
