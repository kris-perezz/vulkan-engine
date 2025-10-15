#include "RenderSystem.h"
#include "GameObject.h"
#include "Log.h"
#include "glm/gtc/constants.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace kopi {
  struct SimplePushConstantData {
    alignas(16) glm::mat2 transform{1.0f};
    alignas(16) glm::vec2 offset;
    alignas(16) glm::vec3 colour;
  };

  RenderSystem::RenderSystem(EngineDevice &device, VkRenderPass renderPass) : m_device{device} {
    createPipelineLayout();
    createPipeline(renderPass);
  }

  RenderSystem::~RenderSystem() {
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
  }

  void RenderSystem::createPipelineLayout() {

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

  void RenderSystem::createPipeline(VkRenderPass renderPass) {
    ASSERT_LOG(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);

    pipelineConfig.renderPass     = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device,
                                            "src/shaders/simple.vert.spv",
                                            "src/shaders/simple.frag.spv",
                                            pipelineConfig);
  }

  void RenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects) {
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