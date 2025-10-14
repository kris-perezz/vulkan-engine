#pragma once

#include "EngineDevice.h"
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
namespace kopi {
  class Model {
  public:
    struct Vertex {
      glm::vec2 position;
      glm::vec3 colour;

      static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
      static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    Model(EngineDevice &device, const std::vector<Vertex> &vertices);
    ~Model();

    Model(const Model &)            = delete;
    Model &operator=(const Model &) = delete;

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

  private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    EngineDevice &m_device;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    uint32_t m_vertexCount;
  };
} // namespace kopi