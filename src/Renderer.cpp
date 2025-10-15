#include "Renderer.h"
#include "EngineDevice.h"
#include "Log.h"
#include "SwapChain.h"
#include "Window.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace kopi {
  Renderer::Renderer(Window &window, EngineDevice &device) : m_window{window}, m_device{device} {
    recreateSwapChain();
    createCommandBuffers();
  }

  Renderer::~Renderer() { freeCommandBuffers(); }

  VkCommandBuffer Renderer::beginFrame() {
    ASSERT_LOG(!m_isFrameStarted, "Can't call beginFrame while already in progress!");
    auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return nullptr;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      LOG_ERROR("Failed to acquire swap chain image!");
      throw std::runtime_error("Failed to acquire swap chain image!");
    }

    m_isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      LOG_ERROR("Failed to begin recording command buffer!");
      throw std::runtime_error("Failed to begin recording command buffer!");
    }

    return commandBuffer;
  }

  void Renderer::endFrame() {
    ASSERT_LOG(m_isFrameStarted, "Can't call endFrame while frame not in progress!");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
    auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_window.wasWindowResized()) {
      m_window.resetWindowResizeFlag();
      recreateSwapChain();
    } else if (result != VK_SUCCESS) {
      LOG_ERROR("Failed to present swap chain image!");
      throw std::runtime_error("Failed to present swap chain image!");
    }

    m_isFrameStarted = false;
    m_currentFrameIndex = (m_currentFrameIndex +1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
  }

  void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    ASSERT_LOG(m_isFrameStarted, "Can't call beginSwapChainRenderPass while already in progress!");
    ASSERT_LOG(commandBuffer == getCurrentCommandBuffer(),
               "Can't begin render pass on a command buffer from a different frame! ");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      LOG_ERROR("Failed to begin recording command buffer!");
      throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = m_swapChain->getRenderPass();
    renderPassInfo.framebuffer       = m_swapChain->getFrameBuffer(m_currentImageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {
        {0.01f, 0.01f, 0.01f, 1.0f}
    };
    clearValues[1].depthStencil    = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(m_swapChain->getSwapChainExtent().width);
    viewport.height   = static_cast<float>(m_swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{
        {0, 0},
        m_swapChain->getSwapChainExtent()
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

  void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    ASSERT_LOG(m_isFrameStarted, "Can't call endSwapChainRenderPass while frame not in progress!");
    ASSERT_LOG(commandBuffer == getCurrentCommandBuffer(),
               "Can't end render pass on a command buffer from a different frame! ");
    vkCmdEndRenderPass(commandBuffer);
  }

  void Renderer::recreateSwapChain() {
    auto extent = m_window.getExtent();

    while (extent.width == 0 || extent.height == 0) {
      extent = m_window.getExtent();
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device.device());

    if (m_swapChain == nullptr) {
      m_swapChain = std::make_unique<SwapChain>(m_device, extent);
    } else {
      std::shared_ptr<SwapChain> oldSwapchain = std::move(m_swapChain);
      m_swapChain = std::make_unique<SwapChain>(m_device, extent, oldSwapchain);

      if (!oldSwapchain->compareSwapFormats(*m_swapChain.get())) {
        LOG_ERROR("Swap chain format image or depth format has changed!");
        throw std::runtime_error("Swap chain format image or depth format has changed!");
      }

    }
  }

  void Renderer::createCommandBuffers() {
    m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = m_device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) !=
        VK_SUCCESS) {
      LOG_ERROR("Failed to allocate command buffers!");
      throw std::runtime_error("Failed to allocate command buffers!");
    }
  }

  void Renderer::freeCommandBuffers() {
    vkFreeCommandBuffers(m_device.device(),
                         m_device.getCommandPool(),
                         static_cast<uint32_t>(m_commandBuffers.size()),
                         m_commandBuffers.data());

    m_commandBuffers.clear();
  }

  VkRenderPass Renderer::getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }

  VkCommandBuffer Renderer::getCurrentCommandBuffer() const {
    ASSERT_LOG(m_isFrameStarted, "Cannot get command buffer when frame not in progress!");
    return m_commandBuffers[m_currentFrameIndex];
  }

  int Renderer::getFrameIndex() const {
    ASSERT_LOG(m_isFrameStarted, "Cannot get frame index when frame not in progress!");
    return m_currentFrameIndex;
  }
} // namespace kopi