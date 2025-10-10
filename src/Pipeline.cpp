#include "Pipeline.h"
#include "Log.h"

#include <fstream>
#include <ios>
#include <stdexcept>

namespace kopi {
  Pipeline::Pipeline(const std::string &vertFilePath, const std::string &fragFilePath) {
    createGraphicsPipeline(vertFilePath, fragFilePath);
  }

  void Pipeline::createGraphicsPipeline(const std::string &vertFilePath,
                                        const std::string &fragFilePath) {

    auto vertCode = readFile(vertFilePath);
    auto fragCode = readFile(fragFilePath);

    LOG_DEBUG("Vertex Shader code size: {}", vertCode.size());
    LOG_DEBUG("Fragment Shader code size: {}", fragCode.size());
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