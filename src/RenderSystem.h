#pragma once

#include "EngineDevice.h"
#include "GameObject.h"
#include "Pipeline.h"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
namespace kopi {
  class RenderSystem {
  public:
    RenderSystem(EngineDevice &device, VkRenderPass renderPass);
    ~RenderSystem();

    RenderSystem(const RenderSystem &)            = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;

    void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects);

  private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);

    EngineDevice &m_device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;
  };
} // namespace kopi