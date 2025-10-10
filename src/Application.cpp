#include "Application.h"
#include "Log.h"
#include "Pipeline.h"
#include "SwapChain.h"
#include "Window.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace kopi {
  Application::Application() {
    loadModels();
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
  }

  Application::~Application() {
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
  }

  void Application::run() {
    while (!m_window.shouldClose()) {
      glfwPollEvents();
      drawFrame();
    }
    vkDeviceWaitIdle(m_device.device());
  }

  void Application::loadModels() {
    std::vector<Model::Vertex> vertices{{{0.0, -0.5}}, {{0.5, 0.5}}, {{-0.5, 0.5}}};

    m_model = std::make_unique<Model>(m_device, vertices);
  }

  void Application::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;
    pipelineLayoutInfo.pSetLayouts            = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;

    if (vkCreatePipelineLayout(m_device.device(),
                               &pipelineLayoutInfo,
                               nullptr,
                               &m_pipelineLayout) != VK_SUCCESS) {
      LOG_ERROR("Failed to create pipeline layout!");
      throw std::runtime_error("Failed to create pipeline layout!");
    }
  }

  void Application::createPipeline() {
    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig, m_swapChain.width(), m_swapChain.height());

    pipelineConfig.renderPass     = m_swapChain.getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline                    = std::make_unique<Pipeline>(m_device,
                                            "src/shaders/simple.vert.spv",
                                            "src/shaders/simple.frag.spv",
                                            pipelineConfig);
  }

  void Application::createCommandBuffers() {
    m_commandBuffers.resize(m_swapChain.imageCount());

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

    for (int i = 0; i < m_commandBuffers.size(); i++) {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
        LOG_ERROR("Failed to begin recording command buffer!");
        throw std::runtime_error("Failed to begin recording command buffer!");
      }

      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass        = m_swapChain.getRenderPass();
      renderPassInfo.framebuffer       = m_swapChain.getFrameBuffer(i);
      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = m_swapChain.getSwapChainExtent();

      std::array<VkClearValue, 2> clearValues{};
      clearValues[0].color           = {0.1f, 0.1f, 0.1f, 1.0f};
      clearValues[1].depthStencil    = {1.0f, 0};
      renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
      renderPassInfo.pClearValues    = clearValues.data();

      vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      m_pipeline->bind(m_commandBuffers[i]);
      m_model->bind(m_commandBuffers[i]);
      m_model->draw(m_commandBuffers[i]);

      vkCmdEndRenderPass(m_commandBuffers[i]);

      if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
        LOG_ERROR("Failed to record command buffer!");
        throw std::runtime_error("Failed to record command buffer!");
      }
    }
  }

  void Application::drawFrame() {
    uint32_t imageIndex;
    auto result = m_swapChain.acquireNextImage(&imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      LOG_ERROR("Failed to acquire swap chain image!");
      throw std::runtime_error("Failed to acquire swap chain image!");
    }
    result = m_swapChain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS) {
      LOG_ERROR("Failed to present swap chain image!");
      throw std::runtime_error("Failed to present swap chain image!");
    }
  }
} // namespace kopi