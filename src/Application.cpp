#include "glm/fwd.hpp"
#include "glm/gtc/constants.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Application.h"
#include "GameObject.h"
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

  struct SimplePushConstantData {
    alignas(16) glm::mat2 transform{1.0f};
    alignas(16) glm::vec2 offset;
    alignas(16) glm::vec3 colour;
  };

  Application::Application() {
    loadGameObjects();
    createPipelineLayout();
    recreateSwapChain();
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

  void Application::loadGameObjects() {
    std::vector<Model::Vertex> vertices{
        {.position = {0.0, -0.5}, .colour = {1.0f, 0.0f, 0.0f}},
        {.position = {0.5, 0.5},  .colour = {0.0f, 1.0f, 0.0f}},
        {.position = {-0.5, 0.5}, .colour = {0.0f, 0.0f, 1.0f}}
    };

    auto m_model = std::make_shared<Model>(m_device, vertices);

    auto triangle                      = GameObject::createGameObject();
    triangle.model                     = m_model;
    triangle.color                     = {0.1f, 0.8f, 0.1f};
    triangle.transform2d.translation.x = 0.2f;
    triangle.transform2d.scale         = {1.0f, 1.5f};
    triangle.transform2d.rotation      = 0.25f * glm::two_pi<float>();

    gameObjects.push_back(std::move(triangle));
  }

  void Application::createPipelineLayout() {

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset     = 0;
    pushConstantRange.size       = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;
    pipelineLayoutInfo.pSetLayouts            = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;

    if (vkCreatePipelineLayout(m_device.device(),
                               &pipelineLayoutInfo,
                               nullptr,
                               &m_pipelineLayout) != VK_SUCCESS) {
      LOG_ERROR("Failed to create pipeline layout!");
      throw std::runtime_error("Failed to create pipeline layout!");
    }
  }

  void Application::createPipeline() {
    ASSERT_LOG(m_swapChain != nullptr, "Cannot create pipeline before swapchain!");
    ASSERT_LOG(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);

    pipelineConfig.renderPass     = m_swapChain->getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device,
                                            "src/shaders/simple.vert.spv",
                                            "src/shaders/simple.frag.spv",
                                            pipelineConfig);
  }

  void Application::recreateSwapChain() {
    auto extent = m_window.getExtent();

    while (extent.width == 0 || extent.height == 0) {
      extent = m_window.getExtent();
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device.device());

    if (m_swapChain == nullptr) {
      m_swapChain = std::make_unique<SwapChain>(m_device, extent);
    } else {
      m_swapChain = std::make_unique<SwapChain>(m_device, extent, std::move(m_swapChain));
      if (m_swapChain->imageCount() != m_commandBuffers.size()) {
        freeCommandBuffers();
        createCommandBuffers();
      }
    }
    // future optimization: if render pass is compatible, do nothing else
    createPipeline();
  }

  void Application::createCommandBuffers() {
    m_commandBuffers.resize(m_swapChain->imageCount());

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

  void Application::freeCommandBuffers() {
    vkFreeCommandBuffers(m_device.device(),
                         m_device.getCommandPool(),
                         static_cast<uint32_t>(m_commandBuffers.size()),
                         m_commandBuffers.data());

    m_commandBuffers.clear();
  }

  void Application::drawFrame() {
    uint32_t imageIndex;
    auto result = m_swapChain->acquireNextImage(&imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      LOG_ERROR("Failed to acquire swap chain image!");
      throw std::runtime_error("Failed to acquire swap chain image!");
    }

    recordCommandBuffer(imageIndex);
    result = m_swapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_window.wasWindowResized()) {
      m_window.resetWindowResizeFlag();
      recreateSwapChain();
      return;
    }
    if (result != VK_SUCCESS) {
      LOG_ERROR("Failed to present swap chain image!");
      throw std::runtime_error("Failed to present swap chain image!");
    }
  }

  void Application::recordCommandBuffer(int imageIndex) {

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
      LOG_ERROR("Failed to begin recording command buffer!");
      throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = m_swapChain->getRenderPass();
    renderPassInfo.framebuffer       = m_swapChain->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {
        {0.01f, 0.01f, 0.01f, 1.0f}
    };
    clearValues[1].depthStencil    = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
    vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

    renderGameObjects(m_commandBuffers[imageIndex]);

    vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

    if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }

  void Application::renderGameObjects(VkCommandBuffer commandBuffer) {
    m_pipeline->bind(commandBuffer);

    for (auto &obj : gameObjects) {
      obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

      SimplePushConstantData push{};
      push.offset    = obj.transform2d.translation;
      push.colour    = obj.color;
      push.transform = obj.transform2d.mat2();

      vkCmdPushConstants(commandBuffer,
                         m_pipelineLayout,
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                         0,
                         sizeof(SimplePushConstantData),
                         &push);
      obj.model->bind(commandBuffer);
      obj.model->draw(commandBuffer);
    }
  }
} // namespace kopi