#pragma once

#include "EngineDevice.h"
#include "SwapChain.h"
#include "Window.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
namespace kopi {
  class Renderer {
  public:
    Renderer(Window &window, EngineDevice &device);
    ~Renderer();

    Renderer(const Renderer &)            = delete;
    Renderer &operator=(const Renderer &) = delete;

    bool isFrameInProgress();

    VkRenderPass getSwapChainRenderPass() const;
    VkCommandBuffer getCurrentCommandBuffer() const;
    int getFrameIndex() const;

    VkCommandBuffer beginFrame();
    void endFrame();

    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

  private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    Window &m_window;
    EngineDevice &m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    std::vector<VkCommandBuffer> m_commandBuffers;

    uint32_t m_currentImageIndex = 0;
    int m_currentFrameIndex      = 0;
    bool m_isFrameStarted        = false;
  };
} // namespace kopi