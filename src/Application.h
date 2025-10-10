#pragma once

#include "EngineDevice.h"
#include "Pipeline.h"
#include "Window.h"
#include <string>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
namespace kopi {
  class Application {
  public:
    static constexpr int WIDTH  = 1280;
    static constexpr int HEIGHT = 720;

    Application();
    ~Application();

    void run();

  private:
    Window m_window{"kopi engine", WIDTH, HEIGHT};
    EngineDevice m_device{m_window};
    Pipeline m_pipeline{m_device,
                        "src/shaders/simple.vert.spv",
                        "src/shaders/simple.frag.spv",
                        Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
  };
} // namespace kopi