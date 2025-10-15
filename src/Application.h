#pragma once

#include "EngineDevice.h"
#include "GameObject.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Window.h"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
namespace kopi {
  class Application {
  public:
    static constexpr int WIDTH  = 1280;
    static constexpr int HEIGHT = 720;

    Application();
    ~Application();

    Application(const Application &)            = delete;
    Application &operator=(const Application &) = delete;

    void run();

  private:
    void loadGameObjects();

    Window m_window{"kopi engine", WIDTH, HEIGHT};
    EngineDevice m_device{m_window};
    Renderer m_renderer{m_window, m_device};

    std::vector<GameObject> m_gameObjects;
  };
} // namespace kopi