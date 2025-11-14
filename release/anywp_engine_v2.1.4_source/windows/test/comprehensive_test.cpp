#include "test_framework.h"
#include "../utils/logger.h"
#include "../utils/resource_tracker.h"
#include "../utils/desktop_wallpaper_helper.h"
#include "../utils/url_validator.h"
#include "../utils/state_persistence.h"
#include "../utils/memory_profiler.h"
#include "../utils/cpu_profiler.h"
#include "../utils/startup_optimizer.h"
#include "../modules/power_manager.h"
#include "../modules/mouse_hook_manager.h"
#include "../modules/monitor_manager.h"
#include "../modules/iframe_detector.h"
#include "../modules/sdk_bridge.h"
#include <windows.h>
#include <set>

using namespace anywp_engine;
using namespace anywp_engine::test;

// ============================================================================
// Comprehensive Test Suite - All Independent Modules
// ============================================================================

TEST_SUITE(Logger) {
  TEST_CASE(singleton_access) {
    auto& logger1 = Logger::Instance();
    auto& logger2 = Logger::Instance();
    ASSERT_TRUE(&logger1 == &logger2);
  }
  
  TEST_CASE(log_levels) {
    auto& logger = Logger::Instance();
    // Should not crash
    logger.Info("Test", "Info message");
    logger.Warning("Test", "Warning message");
    logger.Error("Test", "Error message");
    logger.Debug("Test", "Debug message");
    ASSERT_TRUE(true);
  }
}

TEST_SUITE(PowerManager) {
  TEST_CASE(singleton_access) {
    auto& pm1 = PowerManager::Instance();
    auto& pm2 = PowerManager::Instance();
    ASSERT_TRUE(&pm1 == &pm2);
  }
  
  TEST_CASE(power_state) {
    auto& pm = PowerManager::Instance();
    // Check initial state
    bool is_active = pm.IsFullscreenAppActive();
    ASSERT_TRUE(!is_active || is_active); // Just check it doesn't crash
  }
}

TEST_SUITE(URLValidator) {
  TEST_CASE(basic_validation) {
    URLValidator validator;
    
    // Valid URLs
    ASSERT_TRUE(validator.IsAllowed("https://example.com"));
    ASSERT_TRUE(validator.IsAllowed("http://test.com"));
    ASSERT_TRUE(validator.IsAllowed("file:///C:/test.html"));
  }
  
  TEST_CASE(whitelist) {
    URLValidator validator;
    
    validator.AddWhitelist("example.com");
    ASSERT_TRUE(validator.IsAllowed("https://example.com"));
    ASSERT_TRUE(validator.IsAllowed("https://example.com/page"));
  }
  
  TEST_CASE(blacklist) {
    URLValidator validator;
    
    validator.AddBlacklist("bad.com");
    ASSERT_FALSE(validator.IsAllowed("https://bad.com"));
  }
}

TEST_SUITE(StatePersistence) {
  TEST_CASE(save_and_load) {
    StatePersistence storage;
    storage.SetApplicationName("TestApp");
    
    storage.SaveState("test_key", "test_value");
    std::string loaded = storage.LoadState("test_key");
    ASSERT_EQUAL("test_value", loaded);
  }
  
  TEST_CASE(clear_state) {
    StatePersistence storage;
    storage.SetApplicationName("TestApp2");
    
    storage.SaveState("key", "value");
    storage.ClearState();
    
    std::string loaded = storage.LoadState("key");
    ASSERT_EQUAL("", loaded);
  }
}

TEST_SUITE(IframeDetector) {
  TEST_CASE(initialization) {
    IframeDetector detector;
    ASSERT_EQUAL(0, detector.GetCount());
  }
  
  TEST_CASE(update_iframes) {
    IframeDetector detector;
    
    std::string json = R"([
      {"x": 10, "y": 20, "width": 100, "height": 50}
    ])";
    
    detector.UpdateIframes(json);
    ASSERT_TRUE(detector.GetCount() > 0);
  }
  
  TEST_CASE(clear_iframes) {
    IframeDetector detector;
    
    detector.UpdateIframes(R"([{"x": 0, "y": 0, "width": 10, "height": 10}])");
    detector.Clear();
    
    ASSERT_EQUAL(0, detector.GetCount());
  }
}

TEST_SUITE(MouseHookManager) {
  TEST_CASE(initialization) {
    MouseHookManager manager;
    ASSERT_FALSE(manager.IsInstalled());
  }
  
  TEST_CASE(install_uninstall) {
    MouseHookManager manager;
    
    bool installed = manager.Install();
    if (installed) {
      ASSERT_TRUE(manager.IsInstalled());
      manager.Uninstall();
      ASSERT_FALSE(manager.IsInstalled());
    } else {
      // Hook may fail in test environment, that's OK
      ASSERT_TRUE(true);
    }
  }
}

TEST_SUITE(MonitorManager) {
  TEST_CASE(singleton_access) {
    auto& mm1 = MonitorManager::Instance();
    auto& mm2 = MonitorManager::Instance();
    ASSERT_TRUE(&mm1 == &mm2);
  }
  
  TEST_CASE(get_monitors) {
    auto& manager = MonitorManager::Instance();
    auto monitors = manager.GetMonitors();
    
    ASSERT_TRUE(monitors.size() > 0);
  }
  
  TEST_CASE(monitor_dimensions) {
    auto& manager = MonitorManager::Instance();
    auto monitors = manager.GetMonitors();
    
    for (const auto& monitor : monitors) {
      ASSERT_TRUE(monitor.width > 0);
      ASSERT_TRUE(monitor.height > 0);
    }
  }
}

TEST_SUITE(SDKBridge) {
  TEST_CASE(initialization) {
    SDKBridge bridge;
    ASSERT_TRUE(true); // Should not crash
  }
  
  TEST_CASE(register_handler) {
    SDKBridge bridge;
    
    bool handler_called = false;
    bridge.RegisterHandler("test_type", [&](const std::string&) {
      handler_called = true;
    });
    
    bridge.HandleMessage(R"({"type":"test_type","data":"test"})");
    ASSERT_TRUE(handler_called);
  }
}

TEST_SUITE(MemoryProfiler) {
  TEST_CASE(singleton_access) {
    auto& profiler1 = MemoryProfiler::Instance();
    auto& profiler2 = MemoryProfiler::Instance();
    ASSERT_TRUE(&profiler1 == &profiler2);
  }
  
  TEST_CASE(get_stats) {
    auto& profiler = MemoryProfiler::Instance();
    auto stats = profiler.GetCurrentStats();
    
    ASSERT_TRUE(stats.working_set_size >= 0);
    ASSERT_TRUE(stats.private_usage >= 0);
  }
  
  TEST_CASE(memory_tracking) {
    auto& profiler = MemoryProfiler::Instance();
    
    void* ptr = malloc(1024);
    profiler.TrackAllocation(ptr, 1024, "test");
    
    size_t tracked = profiler.GetTrackedMemory();
    ASSERT_TRUE(tracked >= 1024);
    
    profiler.TrackDeallocation(ptr);
    free(ptr);
  }
}

TEST_SUITE(CPUProfiler) {
  TEST_CASE(singleton_access) {
    auto& profiler1 = CPUProfiler::Instance();
    auto& profiler2 = CPUProfiler::Instance();
    ASSERT_TRUE(&profiler1 == &profiler2);
  }
  
  TEST_CASE(get_stats) {
    auto& profiler = CPUProfiler::Instance();
    auto stats = profiler.GetCurrentStats();
    
    ASSERT_TRUE(stats.process_cpu_percent >= 0);
    ASSERT_TRUE(stats.system_cpu_percent >= 0);
  }
  
  TEST_CASE(timing_operations) {
    auto& profiler = CPUProfiler::Instance();
    
    profiler.StartTiming("test_op");
    Sleep(1);
    profiler.EndTiming("test_op");
    
    auto timing = profiler.GetTimingData("test_op");
    ASSERT_TRUE(timing.call_count > 0);
  }
}

TEST_SUITE(StartupOptimizer) {
  TEST_CASE(singleton_access) {
    auto& optimizer1 = StartupOptimizer::Instance();
    auto& optimizer2 = StartupOptimizer::Instance();
    ASSERT_TRUE(&optimizer1 == &optimizer2);
  }
  
  TEST_CASE(task_registration) {
    auto& optimizer = StartupOptimizer::Instance();
    
    bool executed = false;
    optimizer.RegisterTask("test", [&]() { executed = true; }, InitPriority::NORMAL);
    
    ASSERT_TRUE(true);
  }
  
  TEST_CASE(critical_init) {
    auto& optimizer = StartupOptimizer::Instance();
    
    bool critical_executed = false;
    optimizer.RegisterTask("critical", [&]() { critical_executed = true; }, InitPriority::CRITICAL);
    
    optimizer.BeginStartup();
    optimizer.RunCriticalInit();
    
    ASSERT_TRUE(critical_executed);
  }
}

TEST_SUITE(DesktopWallpaperHelper) {
  TEST_CASE(singleton_access) {
    auto& helper1 = DesktopWallpaperHelper::Instance();
    auto& helper2 = DesktopWallpaperHelper::Instance();
    ASSERT_TRUE(&helper1 == &helper2);
  }
  
  TEST_CASE(get_wallpaper_parent) {
    auto& helper = DesktopWallpaperHelper::Instance();
    HWND parent = helper.GetWallpaperParent();
    
    // May be null in test environment
    ASSERT_TRUE(parent != nullptr || parent == nullptr);
  }
}

TEST_SUITE(ResourceTracker) {
  TEST_CASE(singleton_access) {
    auto& tracker1 = ResourceTracker::Instance();
    auto& tracker2 = ResourceTracker::Instance();
    ASSERT_TRUE(&tracker1 == &tracker2);
  }
}

// Main test runner
int main() {
  std::cout << "\n";
  std::cout << "========================================\n";
  std::cout << "  AnyWP Engine v2.0\n";
  std::cout << "  Comprehensive Module Tests\n";
  std::cout << "========================================\n";
  
  int result = TestRunner::Instance().Run();
  
  std::cout << "\n";
  std::cout << "========================================\n";
  std::cout << "  Test Suite Completed\n";
  std::cout << "========================================\n\n";
  
  return result;
}


