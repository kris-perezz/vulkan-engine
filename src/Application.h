#pragma once

#include "EngineDevice.h"
#include "Model.h"
#include "Pipeline.h"
#include "SwapChain.h"
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
    void loadModels();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();

    Window m_window{"kopi engine", WIDTH, HEIGHT};
    EngineDevice m_device{m_window};
    SwapChain m_swapChain{m_device, m_window.getExtent()};
    std::unique_ptr<Pipeline> m_pipeline;

    VkPipelineLayout m_pipelineLayout;
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::unique_ptr<Model> m_model;
  };
} // namespace kopi