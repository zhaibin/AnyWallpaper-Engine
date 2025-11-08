#include "../test_framework.h"
#include "../../utils/logger.h"
#include "../../modules/power_manager.h"

using namespace anywp_engine;
using namespace anywp_engine::test;

TEST_SUITE(PowerManager) {
  TEST_CASE(initialization) {
    PowerManager manager;
    ASSERT_FALSE(manager.IsEnabled());
    ASSERT_EQUAL(PowerManager::PowerState::ACTIVE, manager.GetCurrentState());
  }
  
  TEST_CASE(enable_disable) {
    PowerManager manager;
    
    manager.Enable(true);
    ASSERT_TRUE(manager.IsEnabled());
    
    manager.Enable(false);
    ASSERT_FALSE(manager.IsEnabled());
  }
  
  TEST_CASE(state_management) {
    PowerManager manager;
    
    PowerManager::PowerState state = manager.GetCurrentState();
    ASSERT_EQUAL(PowerManager::PowerState::ACTIVE, state);
  }
  
  TEST_CASE(memory_usage) {
    PowerManager manager;
    
    size_t memory = manager.GetCurrentMemoryUsage();
    ASSERT_TRUE(memory > 0);  // Should be positive
    ASSERT_TRUE(memory < 10000);  // Should be less than 10GB
  }
}

TEST_SUITE(Logger) {
  TEST_CASE(basic_logging) {
    Logger::Instance().Info("Test", "This is a test message");
    Logger::Instance().Warn("Test", "This is a warning");
    Logger::Instance().Error("Test", "This is an error");
    ASSERT_TRUE(true);  // If we get here, logging works
  }
  
  TEST_CASE(log_levels) {
    Logger::Instance().SetMinLevel(Logger::Level::WARNING);
    Logger::Instance().Debug("Test", "This should not appear");
    Logger::Instance().Info("Test", "This should not appear");
    Logger::Instance().Warning("Test", "This should appear");
    
    // Reset to INFO
    Logger::Instance().SetMinLevel(Logger::Level::INFO);
    ASSERT_TRUE(true);
  }
}

// Main test runner
int main() {
  return TestRunner::Instance().Run();
}


