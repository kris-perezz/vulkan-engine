#include "Application.h"
#include "GravitySystem.h"
#include "Log.h"
#include <exception>
int main() {
  PEREZLOG::Log::init();
  kopi::Application app;
  
  try {
    app.run();
  } catch (const std::exception &e) {
    LOG_ERROR("{}", e.what());
    return -1;
  }

  return 0;
}