
#include "Application.h"
#include "Log.h"
#include <exception>
int main() {
  kopi::Application app;
  
  try {
    app.run();
  } catch (const std::exception &e) {
    LOG_ERROR("{}", e.what());
    return -1;
  }

  return 0;
}