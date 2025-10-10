#pragma once

#include "EngineDevice.h"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace kopi {
  struct PipelineConfigInfo {
    PipelineConfigInfo() = default;

    PipelineConfigInfo(const PipelineConfigInfo &)            = delete;
    PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass         = nullptr;
    uint32_t subpass                = 0;
  };
  class Pipeline {
  public:
    Pipeline(EngineDevice &device,
             const std::string &vertFilePath,
             const std::string &fragFilePath,
             const PipelineConfigInfo &configInfo);

    ~Pipeline();

    Pipeline(const Pipeline &)       = delete;
    void operator=(const Pipeline &) = delete;

    void bind(VkCommandBuffer commandBuffer);

    static void
    defaultPipelineConfigInfo(PipelineConfigInfo &configInfo, uint32_t width, uint32_t height);

  private:
    static std::vector<char> readFile(const std::string &filePath);

    void createGraphicsPipeline(const std::string &vertFilePath,
                                const std::string &fragFilePath,
                                const PipelineConfigInfo &configInfo);

    void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

    EngineDevice &m_device;
    VkPipeline m_graphicsPipeline;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;
  };
} // namespace kopi
