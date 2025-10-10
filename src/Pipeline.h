#pragma once

#include <string>
#include <vector>

namespace kopi {
  class Pipeline {
    public:
    Pipeline(const std::string &vertFilePath, const std::string &fragFilePath);
    //~Pipeline();
    
    private:
    void createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath);
    
    static std::vector<char> readFile(const std::string &filePath);
  };
} // namespace kopi
