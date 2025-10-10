#include "Pipeline.h"
#include "EngineDevice.h"
#include "Log.h"

#include <fstream>
#include <ios>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace kopi {
  Pipeline::Pipeline(EngineDevice &device,
                     const std::string &vertFilePath,
                     const std::string &fragFilePath,
                     const PipelineConfigInfo &configInfo)
      : m_device(device) {
    createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
  }

  PipelineConfigInfo Pipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
    PipelineConfigInfo configInfo{};
    return configInfo;
  }

  void Pipeline::createGraphicsPipeline(const std::string &vertFilePath,
                                        const std::string &fragFilePath,
                                        const PipelineConfigInfo &configInfo) {

    auto vertCode = readFile(vertFilePath);
    auto fragCode = readFile(fragFilePath);

    LOG_DEBUG("Vertex Shader code size: {}", vertCode.size());
    LOG_DEBUG("Fragment Shader code size: {}", fragCode.size());
  }

  void Pipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo createInfo{}; // common pattern with vulkan
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());
    // weird ass alignment??? from char to uint32
    // apparently bypasses this weirdness since the vextor allocates for worst case scenario already?

    if (vkCreateShaderModule(m_device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
      LOG_ERROR("Failed to create shader module!");
      throw std::runtime_error("Failed to create shader module!");
    }
  }

  std::vector<char> Pipeline::readFile(const std::string &filePath) {
    LOG_DEBUG("filepath: {}", filePath);
    std::fstream file(filePath, std::ios::in | std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      LOG_ERROR("failed to open file {}", filePath.data());
      throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), (std::streamsize)fileSize);

    file.close();

    return buffer;
  }

} // namespace kopi