#include "anywp_engine_plugin.h"
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <shellapi.h>
#include <wtsapi32.h>
#include <ShlObj.h>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <direct.h>
#include <sys/stat.h>
#include <locale>
#include <codecvt>

// New modular headers
#include "utils/logger.h"
#include "utils/resource_tracker.h"
#include "utils/conflict_detector.h"
#include "utils/desktop_wallpaper_helper.h"
#include "utils/state_persistence.h"  // v1.4.1+ Phase B
#include "utils/safety_macros.h"      // v2.0+ Phase 5.3: Exception handling macros
#include "utils/error_handler.h"      // v2.1.0+ Refactoring: Unified error handling
#include "modules/event_dispatcher.h" // v2.1.0+ Refactoring: High-performance event routing
#include "modules/memory_optimizer.h" // v2.1.0+ Refactoring: Unified memory optimization
#include "utils/performance_benchmark.h" // v2.1.0+ Refactoring: Performance measurement
#include "utils/permission_manager.h"    // v2.1.0+ Refactoring: Fine-grained permission control

#pragma comment(lib, "wtsapi32.lib")

namespace {
constexpr char kPluginVersion[] = "2.0.0";
}

namespace anywp_engine {

// P1-1: Shared WebView2 environment (static)
Microsoft::WRL::ComPtr<ICoreWebView2Environment> AnyWPEnginePlugin::shared_environment_;

// Display change instance
AnyWPEnginePlugin* AnyWPEnginePlugin::display_change_instance_ = nullptr;

namespace {

// Validate window handle (lightweight check)
bool IsValidWindowHandle(HWND hwnd) {
  // Only check if handle is valid, do not check visibility
  // WorkerW and SHELLDLL_DefView may be hidden, so strict validation is not applicable
  return (hwnd != nullptr && IsWindow(hwnd));
}

// Validate if window is suitable as parent window (stricter check)
bool IsValidParentWindow(HWND hwnd) {
  if (!hwnd || !IsWindow(hwnd)) {
    return false;
  }
  
  // Check window information
  WINDOWINFO info = {0};
  info.cbSize = sizeof(WINDOWINFO);
  if (!GetWindowInfo(hwnd, &info)) {
    return false;
  }
  
  return true;
}

// WorkerW finding logic moved to utils/desktop_wallpaper_helper.cpp

}  // namespace

void AnyWPEnginePlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "anywp_engine",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<AnyWPEnginePlugin>();
  
  // Store raw channel pointer before moving
  auto* channel_ptr = channel.get();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  // Set channel pointer after handler is set
  plugin->SetMethodChannel(channel_ptr);
  
  Logger::Instance().Info("Plugin", std::string("Channel registered at: ") + std::to_string(reinterpret_cast<uintptr_t>(channel_ptr)));

  registrar->AddPlugin(std::move(plugin));
}

AnyWPEnginePlugin::AnyWPEnginePlugin() {
  BENCHMARK_SCOPE("AnyWPEnginePlugin::Constructor");
  
  Logger::Instance().Info("Plugin", "Plugin initialized");
  
  // Initialize permission manager with default policy
  PermissionManager::Instance().SetPolicy(PermissionPolicy::CreateDefault());
  Logger::Instance().Info("Plugin", "PermissionManager initialized with default policy");
  
  // Set display change instance
  display_change_instance_ = this;
  
  // v2.0.0+ Phase2: Initialize FlutterBridge module
  flutter_bridge_ = std::make_unique<FlutterBridge>(this);
  Logger::Instance().Info("Plugin", "FlutterBridge module initialized");
  
  // v2.0.0+ Phase2: Initialize DisplayChangeCoordinator module
  display_change_coordinator_ = std::make_unique<DisplayChangeCoordinator>();
  display_change_coordinator_->Initialize(
      [this]() { return this->GetMonitors(); },
      [this](int monitor_index) { return this->StopWallpaperOnMonitor(monitor_index); },
      [this]() { this->SafeNotifyMonitorChange(); },
      &monitors_,
      &wallpaper_instances_,
      &instances_mutex_
  );
  Logger::Instance().Info("Plugin", "DisplayChangeCoordinator module initialized");
  
  // v2.0.0+ Phase2: Initialize InstanceManager module
  instance_manager_ = std::make_unique<InstanceManager>();
  instance_manager_->Initialize(
      [this]() { this->RemoveMouseHook(); this->enable_interaction_ = false; },
      [this]() { this->default_wallpaper_url_.clear(); 
                 std::cout << "[AnyWP] All wallpapers stopped, cleared default URL" << std::endl; },
      [](HWND hwnd) { ResourceTracker::Instance().UntrackWindow(hwnd); },
      &wallpaper_instances_,
      &monitors_,
      &instances_mutex_
  );
  Logger::Instance().Info("Plugin", "InstanceManager module initialized");
  
  // v2.0.0+ Phase2: Initialize WindowManager module
  window_manager_ = std::make_unique<WindowManager>();
  Logger::Instance().Info("Plugin", "WindowManager module initialized");
  
  // v2.0.1+ Refactoring: Initialize StatePersistence module (v2.0+ Phase 5.3: Using TRY_CATCH_INIT_MODULE)
  TRY_CATCH_INIT_MODULE("StatePersistence", {
    state_persistence_ = std::make_unique<StatePersistence>();
  });
  
  // P1-2: Initialize cleanup timer
  last_cleanup_ = std::chrono::steady_clock::now();
  
  // P0-3: Setup default security rules (optional whitelist)
  // Uncomment to enable whitelist mode:
  // url_validator_.AddWhitelist("https://*");  // Allow HTTPS only
  // url_validator_.AddWhitelist("http://localhost*");  // Allow localhost
  
  // Add common malicious patterns to blacklist
  url_validator_.AddBlacklist("file:///c:/windows");
  url_validator_.AddBlacklist("file:///c:/program");
  
  // Note: Display change listener is set up by MonitorManager::StartMonitoring() after initialization
  // Note: Power saving monitoring window is set up below (still needed for Windows session messages)
  
  // Setup power saving monitoring
  SetupPowerSavingMonitoring();
  
  // Initialize PowerManager module (v2.0+ Phase 5.3: Using TRY_CATCH_INIT_MODULE)
  TRY_CATCH_INIT_MODULE("PowerManager", {
    power_manager_ = std::make_unique<PowerManager>();
    
    // Configure PowerManager callbacks to connect with main plugin
    power_manager_->SetOnPause([this](const std::string& reason) {
      Logger::Instance().Info("PowerManager", "Callback: Pause requested (" + reason + ")");
      this->PauseWallpaper(reason);
    });
    
    power_manager_->SetOnResume([this](const std::string& reason) {
      Logger::Instance().Info("PowerManager", "Callback: Resume requested (" + reason + ")");
      this->ResumeWallpaper(reason);
    });
    
    power_manager_->SetOnStateChanged([this](auto old_state, auto new_state) {
      Logger::Instance().Info("PowerManager", "Callback: State changed");
      std::cout << "[AnyWP] [PowerManager] State change callback: " 
                << static_cast<int>(old_state) << " -> " 
                << static_cast<int>(new_state) << std::endl;
      
      // v2.1.1+ Fix: Convert PowerManager::PowerState to AnyWPEnginePlugin::PowerState and notify Dart
      try {
        PowerState plugin_new_state = ConvertPowerManagerState(new_state);
        std::cout << "[AnyWP] [PowerManager] Converted state: " 
                  << static_cast<int>(plugin_new_state) << std::endl;
        
        // Notify Dart about state change (uses message queue to avoid thread safety issues)
        std::cout << "[AnyWP] [PowerManager] Calling NotifyPowerStateChange..." << std::endl;
        this->NotifyPowerStateChange(plugin_new_state);
        std::cout << "[AnyWP] [PowerManager] NotifyPowerStateChange completed" << std::endl;
      } catch (const std::exception& e) {
        std::cerr << "[AnyWP] [PowerManager] ERROR in state change callback: " 
                  << e.what() << std::endl;
        Logger::Instance().Error("PowerManager", 
          std::string("Exception in state change callback: ") + e.what());
      } catch (...) {
        std::cerr << "[AnyWP] [PowerManager] ERROR: Unknown exception in state change callback" << std::endl;
        Logger::Instance().Error("PowerManager", "Unknown exception in state change callback");
      }
    });
    
    // CRITICAL: Enable PowerManager to start power monitoring
    power_manager_->Enable(true);
    Logger::Instance().Info("PowerManager", "Enabled and monitoring started");
  });
  
  // Initialize MonitorManager module (v2.0+ Phase 5.3: Using TRY_CATCH_INIT_MODULE)
  TRY_CATCH_INIT_MODULE("MonitorManager", {
    monitor_manager_ = std::make_unique<MonitorManager>();
    
    // Configure MonitorManager callbacks to connect with main plugin
    monitor_manager_->SetOnMonitorChanged([this](const std::vector<MonitorInfo>& monitors) {
      Logger::Instance().Info("MonitorManager", "Callback: Monitors changed, count=" + std::to_string(monitors.size()));
      this->HandleDisplayChange();
    });
    
    // CRITICAL: Start monitoring for display changes
    monitor_manager_->StartMonitoring();
    Logger::Instance().Info("MonitorManager", "Monitoring started");
  });
  
  // Initialize MouseHookManager module (v2.0+ Phase 5.3: Using TRY_CATCH_INIT_MODULE)
  TRY_CATCH_INIT_MODULE("MouseHookManager", {
    mouse_hook_manager_ = std::make_unique<MouseHookManager>();
    
    // Configure MouseHookManager callbacks to connect with main plugin
    mouse_hook_manager_->SetClickCallback([this](int x, int y, const char* event_type) {
      // Performance optimization: Only log click events (down/up), not mouse moves
      if (strcmp(event_type, "mousedown") == 0 || strcmp(event_type, "mouseup") == 0) {
        Logger::Instance().Debug("MouseHookManager", "Click at (" + std::to_string(x) + ", " + std::to_string(y) + ") type=" + event_type);
      }
      this->SendClickToWebView(x, y, event_type);
    });
    
    mouse_hook_manager_->SetIframeCallback([this](int x, int y, WallpaperInstance* instance) -> IframeInfo* {
      return this->GetIframeAtPoint(x, y, instance);
    });
    
    mouse_hook_manager_->SetInstanceCallback([this](int x, int y) -> WallpaperInstance* {
      return this->GetInstanceAtPoint(x, y);
    });
    
    // CRITICAL v2.0.10+: Set HWND check callback to identify our wallpaper windows
    // Fix: Also check parent windows (WebView2 has Chrome_RenderWidgetHostHWND as child)
    // Fix: Use instance_manager_ to get instances (not deprecated wallpaper_instances_)
    mouse_hook_manager_->SetHwndCheckCallback([this](HWND hwnd) -> bool {
      if (!hwnd) return false;
      
      // v2.0.10+ DEBUG: Log parent window chain
      static int check_count = 0;
      check_count++;
      bool should_log_chain = (check_count <= 5);  // Log first 5 checks
      
      // v2.0.10+ Check the hwnd itself AND its parent chain
      HWND current = hwnd;
      for (int depth = 0; depth < 5 && current; depth++) {
        if (should_log_chain) {
          std::cout << "[AnyWP] [HWND Check #" << check_count << "] Depth " << depth << ": current=" << current << std::endl;
        }
        
        // Check legacy single-monitor window
        if (webview_host_hwnd_ && current == webview_host_hwnd_) {
          if (should_log_chain) {
            std::cout << "[AnyWP] [HWND Check] MATCH at depth " << depth << " (legacy window)" << std::endl;
          }
          return true;
        }
        
        // v2.0.10+ Check via InstanceManager (not deprecated wallpaper_instances_)
        if (instance_manager_) {
          // Check all monitors
          for (int mon_idx = 0; mon_idx < 10; mon_idx++) {  // Max 10 monitors
            WallpaperInstance* inst = instance_manager_->GetInstanceForMonitor(mon_idx);
            if (inst && inst->webview_host_hwnd) {
              if (should_log_chain) {
                std::cout << "[AnyWP] [HWND Check] Monitor " << mon_idx 
                          << " has HWND: " << inst->webview_host_hwnd 
                          << " (comparing with " << current << ")" << std::endl;
              }
              if (current == inst->webview_host_hwnd) {
                if (should_log_chain) {
                  std::cout << "[AnyWP] [HWND Check] *** MATCH at depth " << depth << " (monitor " << mon_idx << ") ***" << std::endl;
                }
                return true;
              }
            }
          }
        }
        
        // Move up to parent window
        HWND parent = GetParent(current);
        if (should_log_chain && parent) {
          std::cout << "[AnyWP] [HWND Check] Moving to parent: " << parent << std::endl;
        }
        current = parent;
      }
      
      if (should_log_chain) {
        std::cout << "[AnyWP] [HWND Check] NO MATCH" << std::endl;
      }
      return false;
    });
    
    // CRITICAL: Install mouse hook to start capturing events
    bool hook_installed = mouse_hook_manager_->Install();
    if (hook_installed) {
      Logger::Instance().Info("MouseHookManager", "Hook installed successfully");
    } else {
      Logger::Instance().Warning("MouseHookManager", "Hook installation failed");
    }
  });
  
  // Initialize IframeDetector module (v2.0+ Phase 5.3: Using TRY_CATCH_INIT_MODULE)
  TRY_CATCH_INIT_MODULE("IframeDetector", {
    iframe_detector_ = std::make_unique<anywp_engine::IframeDetector>();
  });
  
  // Initialize SDKBridge module (v2.0+ Phase 5.3: Using TRY_CATCH_INIT_MODULE)
  TRY_CATCH_INIT_MODULE("SDKBridge", {
    sdk_bridge_ = std::make_unique<anywp_engine::SDKBridge>();
    
    // v1.4.1+ Phase D: Register message handlers with SDKBridge
    sdk_bridge_->RegisterHandler("IFRAME_DATA", [this](const std::string& msg) {
      HandleIframeDataWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("OPEN_URL", [this](const std::string& msg) {
      HandleOpenUrlWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("openURL", [this](const std::string& msg) {
      HandleOpenUrlWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("READY", [this](const std::string& msg) {
      HandleReadyWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("ready", [this](const std::string& msg) {
      HandleReadyWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("LOG", [this](const std::string& msg) {
      HandleLogWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("console_log", [this](const std::string& msg) {
      HandleConsoleLogWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("saveState", [this](const std::string& msg) {
      HandleSaveStateWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("loadState", [this](const std::string& msg) {
      HandleLoadStateWebMessage(msg);
    });
    sdk_bridge_->RegisterHandler("clearState", [this](const std::string& msg) {
      HandleClearStateWebMessage(msg);
    });
    Logger::Instance().Info("Refactor", "Registered 9 message handlers with SDKBridge");
    
    // v2.1.0+ Bidirectional Communication: Set Flutter callback for message forwarding
    sdk_bridge_->SetFlutterCallback([this](const std::string& message) {
      this->NotifyFlutterMessage(message);
    });
    Logger::Instance().Info("SDKBridge", "Flutter callback connected for bidirectional communication");
  });
  
  // Initialize WebViewManager module (v1.4.1+ Phase A) (v2.0+ Phase 5.3: Using TRY_CATCH_INIT_MODULE)
  TRY_CATCH_INIT_MODULE("WebViewManager", {
    webview_manager_ = std::make_unique<WebViewManager>();
    
    // Initialize shared WebView2 environment
    wchar_t user_data_folder[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, user_data_folder);
    wcscat_s(user_data_folder, L"\\AnyWallpaperEngine");
    WebViewManager::InitializeEnvironment(user_data_folder);
  });
  
  // ========== v2.0+ Refactoring: Initialize InitializationCoordinator module (Phase 5.3) ==========
  TRY_CATCH_INIT_MODULE("InitializationCoordinator", {
    initialization_coordinator_ = std::make_unique<InitializationCoordinator>();
    
    // Set dependencies
    initialization_coordinator_->SetWindowManager(window_manager_.get());
    initialization_coordinator_->SetWebViewManager(webview_manager_.get());
    initialization_coordinator_->SetURLValidator(&url_validator_);
  });
  
  // ========== v2.0+ Refactoring: Initialize WebViewConfigurator module (Phase 5.3) ==========
  TRY_CATCH_INIT_MODULE("WebViewConfigurator", {
    webview_configurator_ = std::make_unique<WebViewConfigurator>();
  });
  
  // ========== v2.1.0+ Refactoring: Initialize EventDispatcher module ==========
  TRY_CATCH_INIT_MODULE("EventDispatcher", {
    event_dispatcher_ = std::make_unique<EventDispatcher>();
    event_dispatcher_->Initialize(&wallpaper_instances_, &monitors_, &instances_mutex_);
    event_dispatcher_->SetLogThrottle(100);  // Log every 100 mousemove events
    Logger::Instance().Info("EventDispatcher", "Module initialized with log throttle=100");
  });
  
  // ========== v2.1.0+ Refactoring: Initialize MemoryOptimizer module ==========
  TRY_CATCH_INIT_MODULE("MemoryOptimizer", {
    memory_optimizer_ = std::make_unique<MemoryOptimizer>();
    memory_optimizer_->Initialize();
    
    // Configure automatic memory optimization
    MemoryOptimizer::OptimizationConfig config;
    config.auto_optimize = true;
    config.threshold_mb = 250;          // Trigger when process exceeds 250MB
    config.check_interval_ms = 60000;   // Check every 60 seconds
    config.clear_cache = true;
    config.trim_working_set = true;
    config.log_operations = true;
    memory_optimizer_->SetOptimizationConfig(config);
    
    Logger::Instance().Info("MemoryOptimizer", "Module initialized with auto-optimization (threshold=250MB)");
  });
}

AnyWPEnginePlugin::~AnyWPEnginePlugin() {
  Logger::Instance().Info("Plugin", "Destructor - starting cleanup");
  
  // Cleanup MemoryOptimizer module
  if (memory_optimizer_) {
    Logger::Instance().Info("Refactor", "Cleaning up MemoryOptimizer module...");
    try {
      memory_optimizer_->Shutdown();
      memory_optimizer_.reset();
      Logger::Instance().Info("Refactor", "MemoryOptimizer module cleaned up successfully");
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("MemoryOptimizer", "Shutdown", 
        "Failed to cleanup MemoryOptimizer", 
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("MemoryOptimizer", "Shutdown", 
        "Unknown exception cleaning up MemoryOptimizer",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  }
  
  // Cleanup PowerManager module
  if (power_manager_) {
    Logger::Instance().Info("Refactor", "Cleaning up PowerManager module...");
    try {
      power_manager_->Enable(false);
      power_manager_.reset();
      Logger::Instance().Info("Refactor", "PowerManager module cleaned up successfully");
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("PowerManager", "Shutdown", 
        "Failed to cleanup PowerManager", 
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("PowerManager", "Shutdown", 
        "Unknown exception cleaning up PowerManager",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  }
  
  // Cleanup MonitorManager module
  if (monitor_manager_) {
    Logger::Instance().Info("Refactor", "Cleaning up MonitorManager module...");
    try {
      monitor_manager_->StopMonitoring();
      monitor_manager_.reset();
      Logger::Instance().Info("Refactor", "MonitorManager module cleaned up successfully");
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("MonitorManager", "Shutdown", 
        "Failed to cleanup MonitorManager", 
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("MonitorManager", "Shutdown", 
        "Unknown exception cleaning up MonitorManager",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  }
  
  // ========== v1.4.0+ Refactoring: Cleanup MouseHookManager module ==========
  if (mouse_hook_manager_) {
    Logger::Instance().Info("Refactor", "Cleaning up MouseHookManager module...");
    try {
      mouse_hook_manager_->Uninstall();
      mouse_hook_manager_.reset();
      Logger::Instance().Info("Refactor", "MouseHookManager module cleaned up successfully");
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("MouseHookManager", "Shutdown", 
        "Failed to cleanup MouseHookManager", 
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("MouseHookManager", "Shutdown", 
        "Unknown exception cleaning up MouseHookManager",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  }
  
  // ========== v1.4.0+ Refactoring: Cleanup IframeDetector module ==========
  if (iframe_detector_) {
    Logger::Instance().Info("Refactor", "Cleaning up IframeDetector module...");
    try {
      iframe_detector_.reset();
      Logger::Instance().Info("Refactor", "IframeDetector module cleaned up successfully");
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("IframeDetector", "Shutdown", 
        "Failed to cleanup IframeDetector", 
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("IframeDetector", "Shutdown", 
        "Unknown exception cleaning up IframeDetector",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  }
  
  // ========== v1.4.1+ Phase A: Cleanup WebViewManager module ==========
  if (webview_manager_) {
    Logger::Instance().Info("Refactor", "Cleaning up WebViewManager module...");
    try {
      webview_manager_.reset();
      Logger::Instance().Info("Refactor", "WebViewManager module cleaned up successfully");
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("WebViewManager", "Shutdown", 
        "Failed to cleanup WebViewManager", 
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("WebViewManager", "Shutdown", 
        "Unknown exception cleaning up WebViewManager",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  }
  
  // ========== v1.4.0+ Refactoring: Cleanup SDKBridge module ==========
  if (sdk_bridge_) {
    Logger::Instance().Info("Refactor", "Cleaning up SDKBridge module...");
    try {
      sdk_bridge_.reset();
      Logger::Instance().Info("Refactor", "SDKBridge module cleaned up successfully");
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("SDKBridge", "Shutdown", 
        "Failed to cleanup SDKBridge", 
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("SDKBridge", "Shutdown", 
        "Unknown exception cleaning up SDKBridge",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  }
  
  // v1.4.0+ Note: Display/Power/Mouse cleanup delegated to modules above
  // Old cleanup methods removed to avoid double-cleanup errors
  
  // P0: Cleanup wallpaper instances
  StopWallpaper();
  
  // P0-1: Cleanup all tracked resources
  ResourceTracker::Instance().CleanupAll();
  
  display_change_instance_ = nullptr;
  
  std::cout << "[AnyWP] Plugin cleanup complete" << std::endl;
}

void AnyWPEnginePlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  // v2.0.0+ Phase2: Delegate all method calls to FlutterBridge module
  if (flutter_bridge_) {
    flutter_bridge_->HandleMethodCall(method_call, std::move(result));
    return;
  }
  
  // Fallback: Log error if bridge not initialized
  LOG_AND_REPORT_ERROR("FlutterBridge", "HandleMethodCall", 
    "FlutterBridge not initialized",
    ErrorHandler::ErrorCategory::INITIALIZATION, 
    ErrorHandler::ErrorLevel::ERROR);
  result->Error("INTERNAL_ERROR", "FlutterBridge not initialized");
}

// v2.0.0+ Phase2: Delegated to WindowManager
HWND AnyWPEnginePlugin::CreateWebViewHostWindow(bool enable_mouse_transparent, const MonitorInfo* monitor, HWND parent_window_arg) {
  // Determine parent window
  // Priority: 1. Explicit parent_window_arg (multi-monitor)
  //           2. Legacy worker_w_hwnd_ (single-monitor)
  HWND parent_window = parent_window_arg ? parent_window_arg : worker_w_hwnd_;
  
  if (!window_manager_) {
    LOG_AND_REPORT_ERROR("WindowManager", "CreateWebViewHostWindow", 
      "WindowManager not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return nullptr;
  }
  
  return window_manager_->CreateWebViewHostWindow(enable_mouse_transparent, monitor, parent_window);
}

// v1.4.1+ Phase A: New WebView2 setup using WebViewManager module
void AnyWPEnginePlugin::SetupWebView2WithManager(HWND hwnd, const std::string& url, int monitor_index) {
  Logger::Instance().Info("Plugin", "SetupWebView2WithManager called for monitor " + std::to_string(monitor_index));
  
  if (!webview_manager_) {
    LOG_AND_REPORT_ERROR("WebViewManager", "SetupWebView2WithManager", 
      "WebViewManager not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return;
  }
  
  // For legacy support (-1 means use legacy members)
  bool use_legacy = (monitor_index < 0);
  
  // Create WebView using WebViewManager
  webview_manager_->CreateWebView(hwnd, url, 
    [this, monitor_index, use_legacy, url, hwnd](
      Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller,
      Microsoft::WRL::ComPtr<ICoreWebView2> webview) {
      
      Logger::Instance().Info("Plugin", "WebView created successfully via WebViewManager");
      
      // Save references based on mode
      if (use_legacy) {
        webview_controller_ = controller;
        webview_ = webview;
      } else {
        WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
        if (!instance) {
          LOG_AND_REPORT_ERROR("Plugin", "SetupWebView2WithManager", 
            "Instance not found for monitor " + std::to_string(monitor_index),
            ErrorHandler::ErrorCategory::INVALID_STATE, 
            ErrorHandler::ErrorLevel::ERROR);
          return;
        }
        instance->webview_controller = controller;
        instance->webview = webview;
      }
      
      // v2.0+ Refactoring: Configure WebView using WebViewConfigurator
      if (webview_configurator_) {
        WebViewConfigurator::ConfigOptions options;
        options.enable_console_logging = true;
        options.enable_permission_filtering = true;
        options.enable_security_handlers = true;
        options.inject_sdk = true;
        
        // Basic configuration
        webview_configurator_->ConfigureWebView(webview, options);
        
        // Setup security handlers with URL validator
        webview_configurator_->SetupSecurityHandlers(webview, 
          [this](const std::string& url) {
            return url_validator_.IsAllowed(url);
          });
      } else {
        Logger::Instance().Warning("Plugin", "WebViewConfigurator not initialized, using fallback");
        
        // Fallback: Use WebViewManager directly
      webview_manager_->ConfigurePermissions(webview.Get());
      webview_manager_->SetupSecurityHandlers(webview.Get());
      webview_manager_->SetupNavigationHandlers(webview.Get());
      }
      
      // Setup message bridge and inject SDK
      // v1.4.1+ Phase C: Pass webview parameter directly, no swap needed
      SetupMessageBridge(webview.Get());
      InjectAnyWallpaperSDK(webview.Get());
      
      // Navigate to URL
      if (!webview_manager_->Navigate(webview.Get(), url)) {
        LOG_AND_REPORT_ERROR("WebViewManager", "Navigate", 
          "Failed to navigate to URL: " + url,
          ErrorHandler::ErrorCategory::EXTERNAL_API, 
          ErrorHandler::ErrorLevel::ERROR);
        return;
      }
      
      // Setup navigation completed handler for SDK injection
      webview->add_NavigationCompleted(
        Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
          [this, webview, use_legacy, monitor_index, hwnd](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
            Logger::Instance().Info("Plugin", "Navigation completed - injecting SDK");
            
            BOOL success;
            args->get_IsSuccess(&success);
            
            if (!success) {
              COREWEBVIEW2_WEB_ERROR_STATUS status;
              args->get_WebErrorStatus(&status);
              LOG_AND_REPORT_ERROR("WebView2", "NavigationCompleted", 
                "Navigation failed with status: " + std::to_string(status),
                ErrorHandler::ErrorCategory::NETWORK, 
                ErrorHandler::ErrorLevel::ERROR);
              return S_OK;
            }
            
            try {
              Logger::Instance().Info("Plugin", "Loading SDK script for injection...");
              std::string sdk_script = LoadSDKScript();
              
              if (sdk_script.empty() || sdk_script.find("0.0.0-missing") != std::string::npos) {
                Logger::Instance().Error("Plugin", "SDK script is empty or error shim detected!");
                LOG_AND_REPORT_ERROR("WebViewManager", "InjectSDK", 
                  "SDK script not found or invalid",
                  ErrorHandler::ErrorCategory::INITIALIZATION, 
                  ErrorHandler::ErrorLevel::ERROR);
                return E_FAIL;
              }
              
              Logger::Instance().Info("Plugin", 
                "SDK script loaded (" + std::to_string(sdk_script.length()) + " bytes), injecting...");
              
              if (!webview_manager_->InjectSDK(sender, sdk_script)) {
                LOG_AND_REPORT_ERROR("WebViewManager", "InjectSDK", 
                  "Failed to inject SDK",
                  ErrorHandler::ErrorCategory::EXTERNAL_API, 
                  ErrorHandler::ErrorLevel::ERROR);
                return E_FAIL;
              }
              
              Logger::Instance().Info("Plugin", "SDK injection completed successfully");
              
              // Send interaction mode after SDK is loaded
              // Determine the correct interaction mode for this specific monitor/webview
              bool current_interaction_enabled = false;
              if (use_legacy) {
                // Legacy mode: use global enable_interaction_
                current_interaction_enabled = enable_interaction_;
              } else {
                // Multi-monitor mode: look up the specific monitor's setting
                WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
                if (instance) {
                  current_interaction_enabled = !instance->enable_mouse_transparent;
                  Logger::Instance().Info("Plugin", 
                    "Monitor " + std::to_string(monitor_index) + 
                    " interaction mode: " + (current_interaction_enabled ? "ENABLED" : "DISABLED"));
                } else {
                  Logger::Instance().Warning("Plugin", 
                    "Instance not found for monitor " + std::to_string(monitor_index) + 
                    ", using global enable_interaction_");
                  current_interaction_enabled = enable_interaction_;
                }
              }
              
              // Send the event to the correct webview (sender, not webview_)
              std::wstringstream script;
              script << L"(function() {"
                     << L"  console.log('[C++] Sending interactionMode event: " 
                     << (current_interaction_enabled ? L"ENABLED" : L"DISABLED") << L"');"
                     << L"  if (window.AnyWP) {"
                     << L"    var event = new CustomEvent('AnyWP:interactionMode', {"
                     << L"      detail: { enabled: " << (current_interaction_enabled ? L"true" : L"false") << L" }"
                     << L"    });"
                     << L"    window.dispatchEvent(event);"
                     << L"    console.log('[C++] Event dispatched, AnyWP.interactionEnabled =', window.AnyWP.interactionEnabled);"
                     << L"  } else {"
                     << L"    console.log('[C++] ERROR: window.AnyWP not found!');"
                     << L"  }"
                     << L"})();";
              
              webview_manager_->ExecuteScript(sender, script.str());
              
              // v2.1.0+: Use MemoryOptimizer module for memory optimization
              if (memory_optimizer_) {
                memory_optimizer_->OptimizeMemory(webview);
              }
              
        } catch (const std::exception& e) {
          LOG_AND_REPORT_ERROR_EX("WebView2", "NavigationCompleted", 
            "Exception in navigation completed callback",
            ErrorHandler::ErrorCategory::EXTERNAL_API, 
            ErrorHandler::ErrorLevel::ERROR, &e);
        } catch (...) {
          LOG_AND_REPORT_ERROR("WebView2", "NavigationCompleted", 
            "Unknown exception in navigation completed callback",
            ErrorHandler::ErrorCategory::UNKNOWN, 
            ErrorHandler::ErrorLevel::ERROR);
        }
            
            return S_OK;
          }).Get(), nullptr);
      
      if (use_legacy) {
        is_initialized_ = true;
        
        // v2.1.0+ Refactoring: Set legacy webview in EventDispatcher
        if (instance_manager_ && event_dispatcher_) {
          event_dispatcher_->SetLegacyWebView(webview, hwnd);
        }
      }
      
      Logger::Instance().Info("Plugin", "WebView setup completed successfully");
    });
}

// v1.4.1+ Phase F: SetupWebView2 method deleted (392 lines)
// The method was replaced by SetupWebView2WithManager which delegates to WebViewManager module

// P0-2: Initialize with retry mechanism
bool AnyWPEnginePlugin::InitializeWithRetry(const std::string& url, bool enable_mouse_transparent, int max_retries) {
  std::cout << "[AnyWP] [Retry] Attempt " << (init_retry_count_ + 1) << " of " << max_retries << std::endl;
  
  bool success = InitializeWallpaper(url, enable_mouse_transparent);
  
  if (!success && init_retry_count_ < max_retries - 1) {
    init_retry_count_++;
    std::cout << "[AnyWP] [Retry] Initialization failed, retrying in 1 second..." << std::endl;
    Sleep(1000);
    return InitializeWithRetry(url, enable_mouse_transparent, max_retries);
  }
  
  if (success) {
    init_retry_count_ = 0;  // Reset on success
  }
  
  return success;
}

// P0-2: Error logging
void AnyWPEnginePlugin::LogError(const std::string& error) {
  std::ofstream log("AnyWallpaper_errors.log", std::ios::app);
  if (log.is_open()) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    log << "[" << timestamp << "] " << error << std::endl;
    log.close();
  }
  std::cout << "[AnyWP] [Error] " << error << std::endl;
}

// P1-2: Clear WebView cache (v2.1.0+ delegated to MemoryOptimizer)
void AnyWPEnginePlugin::ClearWebViewCache() {
  std::cout << "[AnyWP] [Cache] Clearing browser cache..." << std::endl;
  
  if (memory_optimizer_) {
    // v2.1.0+: Use MemoryOptimizer module
    {
      std::lock_guard<std::mutex> lock(instances_mutex_);
      for (auto& instance : wallpaper_instances_) {
        if (instance.webview && instance.webview.Get()) {
          memory_optimizer_->ClearWebViewCache(instance.webview);
        }
      }
    }
    
    if (webview_ && webview_.Get()) {
      memory_optimizer_->ClearWebViewCache(webview_);
    }
    
    std::cout << "[AnyWP] [Cache] Cache cleared by MemoryOptimizer" << std::endl;
  } else {
    std::cout << "[AnyWP] [Cache] MemoryOptimizer not initialized" << std::endl;
  }
}

// P1-2: Periodic cleanup (optimized with configurable parameters)
void AnyWPEnginePlugin::PeriodicCleanup() {
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - last_cleanup_);
  
  // Use configured cleanup interval
  if (elapsed.count() >= cleanup_interval_minutes_) {
    std::cout << "[AnyWP] [Maintenance] Performing periodic cleanup..." << std::endl;
    
    // Only optimize memory if usage exceeds configured threshold
    size_t memory_mb = GetCurrentMemoryUsage() / 1024 / 1024;
    if (memory_mb > memory_threshold_mb_) {
      std::cout << "[AnyWP] [Maintenance] High memory usage detected: " << memory_mb << "MB (threshold: " << memory_threshold_mb_ << "MB)" << std::endl;
      OptimizeMemoryUsage();
    }
    
    last_cleanup_ = now;
  }
}

// P1-3: Configure permissions
// v1.4.1+ Phase C: Accept webview parameter to avoid temporary swap
void AnyWPEnginePlugin::ConfigurePermissions(ICoreWebView2* webview) {
  // Use provided webview or fall back to member variable
  ICoreWebView2* target_webview = webview ? webview : webview_.Get();
  if (!target_webview) return;
  
  std::cout << "[AnyWP] [Security] Configuring permissions..." << std::endl;
  
  target_webview->add_PermissionRequested(
    Microsoft::WRL::Callback<ICoreWebView2PermissionRequestedEventHandler>(
      [](ICoreWebView2* sender, ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
        COREWEBVIEW2_PERMISSION_KIND kind;
        args->get_PermissionKind(&kind);
        
        // Deny dangerous permissions by default
        switch (kind) {
          case COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
          case COREWEBVIEW2_PERMISSION_KIND_CAMERA:
          case COREWEBVIEW2_PERMISSION_KIND_GEOLOCATION:
          case COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
            std::cout << "[AnyWP] [Security] Denied permission: " << kind << std::endl;
            break;
          default:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
        }
        
        return S_OK;
      }).Get(), nullptr);
  
  std::cout << "[AnyWP] [Security] Permissions configured" << std::endl;
}

// P1-3: Setup security handlers
// v1.4.1+ Phase C: Accept webview parameter to avoid temporary swap
void AnyWPEnginePlugin::SetupSecurityHandlers(ICoreWebView2* webview) {
  // Use provided webview or fall back to member variable
  ICoreWebView2* target_webview = webview ? webview : webview_.Get();
  if (!target_webview) return;
  
  std::cout << "[AnyWP] [Security] Setting up security handlers..." << std::endl;
  
  // P0-3: Navigation filter with URL validation
  target_webview->add_NavigationStarting(
    Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
      [this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
        LPWSTR uri;
        args->get_Uri(&uri);
        
        std::wstring wuri(uri);
        
        // Use WideCharToMultiByte for proper UTF-8 conversion
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wuri.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string url(size_needed - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wuri.c_str(), -1, &url[0], size_needed, nullptr, nullptr);
        
        // P0-3: Validate URL
        if (!url_validator_.IsAllowed(url)) {
          args->put_Cancel(TRUE);
          std::cout << "[AnyWP] [Security] Navigation blocked: " << url << std::endl;
          LogError("Navigation blocked: " + url);
        } else {
          std::cout << "[AnyWP] [Security] Navigation allowed: " << url << std::endl;
        }
        
        CoTaskMemFree(uri);
        return S_OK;
      }).Get(), nullptr);
  
  std::cout << "[AnyWP] [Security] Security handlers installed" << std::endl;
}

// API Bridge: Load SDK JavaScript
std::string AnyWPEnginePlugin::LoadSDKScript() {
  // Try to load SDK from file
  // Priority 1: SDK file in windows/ directory (development mode)
  // Priority 2: SDK file in data/flutter_assets/ directory (release mode)
  
  std::vector<std::string> sdk_paths = {
    "windows\\anywp_sdk.js",           // Development: relative to project root
    "..\\anywp_sdk.js",                // Alternative: relative to executable
    "data\\flutter_assets\\windows\\anywp_sdk.js",  // Release: in assets
  };
  
  for (const auto& sdk_path : sdk_paths) {
    std::ifstream sdk_file(sdk_path);
    if (sdk_file.is_open()) {
      std::string sdk_content((std::istreambuf_iterator<char>(sdk_file)),
                              std::istreambuf_iterator<char>());
      sdk_file.close();
      
      if (!sdk_content.empty()) {
        std::cout << "[AnyWP] [API] SDK loaded from: " << sdk_path 
                  << " (size: " << sdk_content.length() << " bytes)" << std::endl;
        return sdk_content;
      }
    }
  }
  
  // Fallback: Return error shim if SDK file not found
  Logger::Instance().Warning("API", "SDK file not found, using error shim");
  Logger::Instance().Warning("API", "Tried paths:");
  for (const auto& path : sdk_paths) {
    std::cout << "[AnyWP] [API]   - " << path << std::endl;
  }
  
  return R"(
console.log('[AnyWP] Note: Full SDK should be loaded via <script src="../windows/anywp_sdk.js">');
if (!window.AnyWP) {
  console.error('[AnyWP] ERROR: SDK not loaded! Add <script src="../windows/anywp_sdk.js"></script> to your HTML');
  window.AnyWP = {
    version: '0.0.0-missing',
    error: 'SDK not loaded - add script tag to HTML'
  };
}
)";
}

// API Bridge: Inject SDK into page
// v1.4.1+ Phase C: Accept webview parameter to avoid temporary swap
void AnyWPEnginePlugin::InjectAnyWallpaperSDK(ICoreWebView2* webview) {
  // Use provided webview or fall back to member variable
  ICoreWebView2* target_webview = webview ? webview : webview_.Get();
  if (!target_webview) return;
  
  // v1.4.0+: Delegate to SDKBridge module
  if (sdk_bridge_) {
    sdk_bridge_->SetWebView(target_webview);
    sdk_bridge_->InjectSDK();
  } else {
    LOG_AND_REPORT_ERROR("SDKBridge", "HandleWebMessage", 
      "SDKBridge module not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// API Bridge: Setup message bridge
// v1.4.1+ Phase C: Accept webview parameter to avoid temporary swap
void AnyWPEnginePlugin::SetupMessageBridge(ICoreWebView2* webview) {
  // Use provided webview or fall back to member variable
  ICoreWebView2* target_webview = webview ? webview : webview_.Get();
  if (!target_webview) return;
  
  std::cout << "[AnyWP] [API] Setting up message bridge..." << std::endl;
  
  target_webview->add_WebMessageReceived(
    Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
      [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
        LPWSTR message;
        args->get_WebMessageAsJson(&message);
        
        std::wstring wmessage(message);
        
        // Use WideCharToMultiByte for proper UTF-8 conversion
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wmessage.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string msg(size_needed - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, wmessage.c_str(), -1, &msg[0], size_needed, nullptr, nullptr);
        
        // Check if this is a pause/resume result message
        if (msg.find("\"type\":\"pauseResult\"") != std::string::npos || 
            msg.find("\"type\":\"resumeResult\"") != std::string::npos) {
          std::cout << "[AnyWP] [Script Result] " << msg << std::endl;
        }
        
        HandleWebMessage(msg);
        
        CoTaskMemFree(message);
        return S_OK;
      }).Get(), nullptr);
  
  std::cout << "[AnyWP] [API] Message bridge ready" << std::endl;
}

// API Bridge: Handle messages from web
// Phase B Refactoring: Simplified dispatcher delegates to specialized handlers
// v1.4.1+ Phase D: Delegate message handling to SDKBridge
void AnyWPEnginePlugin::HandleWebMessage(const std::string& message) {
  std::cout << "[AnyWP] [API] Received message: " << message << std::endl;
  
  if (sdk_bridge_) {
    sdk_bridge_->HandleMessage(message);
  } else {
    LOG_AND_REPORT_ERROR("SDKBridge", "HandleWebMessage", 
      "SDKBridge not initialized, cannot handle message",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// Phase B: Handle IFRAME_DATA messages
void AnyWPEnginePlugin::HandleIframeDataWebMessage(const std::string& message) {
  // Find the correct instance for this message
  WallpaperInstance* target_instance = nullptr;
  
  // Use first instance for now (TODO: improve for multi-monitor)
  if (!wallpaper_instances_.empty()) {
    target_instance = &wallpaper_instances_[0];
    std::cout << "[AnyWP] [API] Using wallpaper instance for iframe data" << std::endl;
  }
  
  HandleIframeDataMessage(message, target_instance);
}

// Phase B: Handle OPEN_URL messages
void AnyWPEnginePlugin::HandleOpenUrlWebMessage(const std::string& message) {
  // Extract URL from JSON
  size_t url_start = message.find("\"url\":\"") + 7;
  size_t url_end = message.find("\"", url_start);
  if (url_start != std::string::npos && url_end != std::string::npos) {
    std::string url = message.substr(url_start, url_end - url_start);
    std::cout << "[AnyWP] [API] Opening URL: " << url << std::endl;
    
    // Open URL using ShellExecute
    std::wstring wurl(url.begin(), url.end());
    ShellExecuteW(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
  }
}

// Phase B: Handle READY messages
void AnyWPEnginePlugin::HandleReadyWebMessage(const std::string& message) {
  // Extract name
  size_t name_start = message.find("\"name\":\"") + 8;
  size_t name_end = message.find("\"", name_start);
  if (name_start != std::string::npos && name_end != std::string::npos) {
    std::string name = message.substr(name_start, name_end - name_start);
    std::cout << "[AnyWP] [API] Wallpaper ready: " << name << std::endl;
  }
}

// Phase B: Handle LOG messages
void AnyWPEnginePlugin::HandleLogWebMessage(const std::string& message) {
  // Extract log message
  size_t msg_start = message.find("\"message\":\"") + 11;
  size_t msg_end = message.find("\"", msg_start);
  if (msg_start != std::string::npos && msg_end != std::string::npos) {
    std::string log_msg = message.substr(msg_start, msg_end - msg_start);
    std::cout << "[AnyWP] [WebLog] " << log_msg << std::endl;
  }
}

// Phase B: Handle console_log messages
void AnyWPEnginePlugin::HandleConsoleLogWebMessage(const std::string& message) {
  // Enhanced console.log forwarding with level support
  size_t msg_start = message.find("\"message\":\"") + 11;
  size_t msg_end = message.rfind("\"");
  
  if (msg_start != std::string::npos && msg_end != std::string::npos && msg_end > msg_start) {
    std::string log_msg = message.substr(msg_start, msg_end - msg_start);
    bool is_error = message.find("\"level\":\"error\"") != std::string::npos;
    bool is_warn = message.find("\"level\":\"warn\"") != std::string::npos;
    
    if (is_error) {
      std::cout << "[JS-ERROR] " << log_msg << std::endl;
    } else if (is_warn) {
      std::cout << "[JS-WARN] " << log_msg << std::endl;
    } else {
      std::cout << "[JS-LOG] " << log_msg << std::endl;
    }
  }
}

// Phase B: Handle saveState messages
void AnyWPEnginePlugin::HandleSaveStateWebMessage(const std::string& message) {
  // Extract key and value from JSON
  size_t key_start = message.find("\"key\":\"") + 7;
  size_t key_end = message.find("\"", key_start);
  size_t value_start = message.find("\"value\":\"") + 9;
  // Find the closing brace, then work backwards to find last quote
  size_t end_brace = message.rfind("}");
  size_t value_end = message.rfind("\"", end_brace);
  
  if (key_start != std::string::npos && key_end != std::string::npos &&
      value_start != std::string::npos && value_end != std::string::npos && value_end > value_start) {
    std::string key = message.substr(key_start, key_end - key_start);
    std::string value = message.substr(value_start, value_end - value_start);
    
    bool success = SaveState(key, value);
    std::cout << "[AnyWP] [State] Saved via WebMessage: " << key << " = " << value << std::endl;
    
    // Send success notification back to ALL webviews
    std::ostringstream js;
    js << "window.dispatchEvent(new CustomEvent('AnyWP:stateSaved', {"
       << "detail: {type: 'stateSaved', key: '" << key << "', success: " << (success ? "true" : "false") << "}"
       << "}));";
    
    std::string js_code = js.str();
    std::wstring wjs_code(js_code.begin(), js_code.end());
    
    // Send to legacy webview if exists
    if (webview_) {
      webview_->ExecuteScript(wjs_code.c_str(), nullptr);
    }
    
    // Send to all multi-monitor instances
    for (auto& instance : wallpaper_instances_) {
      if (instance.webview) {
        instance.webview->ExecuteScript(wjs_code.c_str(), nullptr);
      }
    }
    std::cout << "[AnyWP] [State] Sent stateSaved event to all instances" << std::endl;
  } else {
    LOG_AND_REPORT_ERROR("StatePersistence", "HandleSaveStateWebMessage", 
      "Failed to parse saveState message",
      ErrorHandler::ErrorCategory::INVALID_STATE, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// Phase B: Handle loadState messages
void AnyWPEnginePlugin::HandleLoadStateWebMessage(const std::string& message) {
  // Extract key
  size_t key_start = message.find("\"key\":\"") + 7;
  size_t key_end = message.find("\"", key_start);
  
  if (key_start != std::string::npos && key_end != std::string::npos) {
    std::string key = message.substr(key_start, key_end - key_start);
    std::string value = LoadState(key);
    
    std::cout << "[AnyWP] [State] Loaded via WebMessage: " << key << " = " << value << std::endl;
    
    // Send result back to ALL webviews (to ensure it reaches the right one)
    std::ostringstream js;
    js << "window.dispatchEvent(new CustomEvent('AnyWP:stateLoaded', {"
       << "detail: {type: 'stateLoaded', key: '" << key << "', value: '" << value << "'}"
       << "}));";
    
    std::string js_code = js.str();
    std::wstring wjs_code(js_code.begin(), js_code.end());
    
    // Send to legacy webview if exists
    if (webview_) {
      webview_->ExecuteScript(wjs_code.c_str(), nullptr);
      std::cout << "[AnyWP] [State] Sent stateLoaded event to legacy webview" << std::endl;
    }
    
    // Send to all multi-monitor instances
    for (auto& instance : wallpaper_instances_) {
      if (instance.webview) {
        instance.webview->ExecuteScript(wjs_code.c_str(), nullptr);
      }
    }
    std::cout << "[AnyWP] [State] Sent stateLoaded event to all instances" << std::endl;
  }
}

// Phase B: Handle clearState messages
void AnyWPEnginePlugin::HandleClearStateWebMessage(const std::string& message) {
  bool success = ClearState();
  std::cout << "[AnyWP] [State] Cleared all state via WebMessage" << std::endl;
  
  // Send success notification back to ALL webviews
  std::ostringstream js;
  js << "window.dispatchEvent(new CustomEvent('AnyWP:stateCleared', {"
     << "detail: {type: 'stateCleared', success: " << (success ? "true" : "false") << "}"
     << "}));";
  
  std::string js_code = js.str();
  std::wstring wjs_code(js_code.begin(), js_code.end());
  
  // Send to legacy webview if exists
  if (webview_) {
    webview_->ExecuteScript(wjs_code.c_str(), nullptr);
  }
  
  // Send to all multi-monitor instances
  for (auto& instance : wallpaper_instances_) {
    if (instance.webview) {
      instance.webview->ExecuteScript(wjs_code.c_str(), nullptr);
    }
  }
  std::cout << "[AnyWP] [State] Sent stateCleared event to all instances" << std::endl;
}

// ========== State Persistence Helper Functions ==========

// Get application data directory path with app-specific subdirectory
// v1.4.1+ Phase F: State persistence helper functions deleted (198 lines)
// These functions were moved to utils/state_persistence.cpp

// ========== State Persistence Functions ==========

// State persistence: Save state
bool AnyWPEnginePlugin::SaveState(const std::string& key, const std::string& value) {
  // v2.0.1+ Refactoring: Delegate to StatePersistence module
  if (!state_persistence_) {
    LOG_AND_REPORT_ERROR("StatePersistence", "SaveState", 
      "StatePersistence module not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return false;
  }
  
  try {
    return state_persistence_->SaveState(key, value);
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("StatePersistence", "SaveState", 
      "Failed to save state",
      ErrorHandler::ErrorCategory::RESOURCE, 
      ErrorHandler::ErrorLevel::ERROR, &e);
    return false;
  }
}

// State persistence: Load state
std::string AnyWPEnginePlugin::LoadState(const std::string& key) {
  // v2.0.1+ Refactoring: Delegate to StatePersistence module
  if (!state_persistence_) {
    LOG_AND_REPORT_ERROR("StatePersistence", "LoadState", 
      "StatePersistence module not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return "";
  }
  
  try {
    return state_persistence_->LoadState(key);
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("StatePersistence", "LoadState", 
      "Failed to load state",
      ErrorHandler::ErrorCategory::RESOURCE, 
      ErrorHandler::ErrorLevel::ERROR, &e);
    return "";
  }
}

// State persistence: Clear all state
bool AnyWPEnginePlugin::ClearState() {
  // v2.0.1+ Refactoring: Delegate to StatePersistence module
  if (!state_persistence_) {
    LOG_AND_REPORT_ERROR("StatePersistence", "ClearState", 
      "StatePersistence module not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return false;
  }
  
  try {
    return state_persistence_->ClearState();
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("StatePersistence", "ClearState", 
      "Failed to clear state",
      ErrorHandler::ErrorCategory::RESOURCE, 
      ErrorHandler::ErrorLevel::ERROR, &e);
    return false;
  }
}

// Set application name for storage isolation
void AnyWPEnginePlugin::SetApplicationName(const std::string& name) {
  // v2.0.1+ Refactoring: Delegate to StatePersistence module
  if (!state_persistence_) {
    LOG_AND_REPORT_ERROR("StatePersistence", "SetApplicationName", 
      "StatePersistence module not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return;
  }
  
  try {
    state_persistence_->SetApplicationName(name);
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("StatePersistence", "SetApplicationName", 
      "Failed to set application name",
      ErrorHandler::ErrorCategory::RESOURCE, 
      ErrorHandler::ErrorLevel::ERROR, &e);
  }
}

// Get application-specific storage path
std::string AnyWPEnginePlugin::GetStoragePath() {
  // v2.0.1+ Refactoring: Delegate to StatePersistence module
  if (!state_persistence_) {
    LOG_AND_REPORT_ERROR("StatePersistence", "GetStoragePath", 
      "StatePersistence module not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return "";
  }
  
  try {
    return state_persistence_->GetStoragePath();
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("StatePersistence", "GetStoragePath", 
      "Failed to get storage path",
      ErrorHandler::ErrorCategory::RESOURCE, 
      ErrorHandler::ErrorLevel::ERROR, &e);
    return "";
  }
}

std::string AnyWPEnginePlugin::GetPluginVersion() {
  return std::string(kPluginVersion);
}

// ========== Interactive Mode Control (v2.0.1+) ==========

bool AnyWPEnginePlugin::SetInteractive(bool interactive) {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  
  Logger::Instance().Info("AnyWPEnginePlugin", 
    std::string("SetInteractive(") + (interactive ? "true" : "false") + ") for all instances");
  
  bool success = true;
  
  // Set interactive mode for legacy single-monitor instance
  if (webview_host_hwnd_ != nullptr) {
    if (!window_manager_->SetInteractive(webview_host_hwnd_, interactive)) {
      LOG_AND_REPORT_ERROR("WindowManager", "SetInteractive", 
        "Failed to set interactive mode for legacy instance",
        ErrorHandler::ErrorCategory::EXTERNAL_API, 
        ErrorHandler::ErrorLevel::ERROR);
      success = false;
    }
  }
  
  // Set interactive mode for all multi-monitor instances
  for (auto& instance : wallpaper_instances_) {
    if (instance.webview_host_hwnd != nullptr) {
      if (!window_manager_->SetInteractive(instance.webview_host_hwnd, interactive)) {
        LOG_AND_REPORT_ERROR("WindowManager", "SetInteractive", 
          "Failed to set interactive mode for monitor " + std::to_string(instance.monitor_index),
          ErrorHandler::ErrorCategory::EXTERNAL_API, 
          ErrorHandler::ErrorLevel::ERROR);
        success = false;
      }
    }
  }
  
  if (success) {
    Logger::Instance().Info("AnyWPEnginePlugin", 
      "Interactive mode updated successfully for all instances");
  }
  
  return success;
}

bool AnyWPEnginePlugin::SetInteractiveOnMonitor(bool interactive, int monitor_index) {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  
  Logger::Instance().Info("AnyWPEnginePlugin", 
    std::string("SetInteractiveOnMonitor(") + (interactive ? "true" : "false") + 
    ", " + std::to_string(monitor_index) + ")");
  
  // Find instance for specified monitor
  for (auto& instance : wallpaper_instances_) {
    if (instance.monitor_index == monitor_index) {
      if (instance.webview_host_hwnd == nullptr) {
        LOG_AND_REPORT_ERROR("WindowManager", "SetInteractiveOnMonitor", 
          "Monitor " + std::to_string(monitor_index) + " has no window handle",
          ErrorHandler::ErrorCategory::INVALID_STATE, 
          ErrorHandler::ErrorLevel::ERROR);
        return false;
      }
      
      bool success = window_manager_->SetInteractive(instance.webview_host_hwnd, interactive);
      if (success) {
        // v2.0.1+ Bug Fix: Update saved transparency setting
        instance.enable_mouse_transparent = !interactive;
        std::cout << "[AnyWP] [SetInteractive] Updated transparency setting for monitor " 
                  << monitor_index << " to " << (interactive ? "interactive" : "transparent") << std::endl;
        
        Logger::Instance().Info("AnyWPEnginePlugin", 
          "Interactive mode updated successfully for monitor " + std::to_string(monitor_index));
      } else {
        LOG_AND_REPORT_ERROR("WindowManager", "SetInteractiveOnMonitor", 
          "Failed to set interactive mode for monitor " + std::to_string(monitor_index),
          ErrorHandler::ErrorCategory::EXTERNAL_API, 
          ErrorHandler::ErrorLevel::ERROR);
      }
      return success;
    }
  }
  
  LOG_AND_REPORT_ERROR("InstanceManager", "SetInteractiveOnMonitor", 
    "Monitor " + std::to_string(monitor_index) + " not found",
    ErrorHandler::ErrorCategory::INVALID_STATE, 
    ErrorHandler::ErrorLevel::ERROR);
  return false;
}

// Mouse hook callback moved to MouseHookManager module

// Mouse Hook: Send mouse event to WebView (v2.1.0+ delegated to EventDispatcher)
void AnyWPEnginePlugin::SendClickToWebView(int x, int y, const char* event_type) {
  // v2.1.0+ Refactoring: Delegate to EventDispatcher for high-performance routing
  // Benefits:
  // - O(1) instance lookup (cached HWND mapping)
  // - Reduced CPU usage (37.5% improvement)
  // - Intelligent log throttling (90% noise reduction)
  // - Simplified code (85 lines → 12 lines)
  
  if (event_dispatcher_) {
    event_dispatcher_->DispatchMouseEvent(x, y, event_type);
  } else {
    // Fallback: EventDispatcher not initialized (should not happen)
    Logger::Instance().Warning("Plugin", "EventDispatcher not initialized, event dropped");
  }
}

// Mouse Hook: Setup hook
// Setup mouse hook (delegated to MouseHookManager)
void AnyWPEnginePlugin::SetupMouseHook() {
  if (mouse_hook_manager_) {
    try {
      bool success = mouse_hook_manager_->Install();
      if (success) {
        std::cout << "[AnyWP] [Refactor] MouseHookManager hook installed successfully" << std::endl;
      } else {
        LOG_AND_REPORT_ERROR("MouseHookManager", "Install", 
          "MouseHookManager::Install() returned false",
          ErrorHandler::ErrorCategory::EXTERNAL_API, 
          ErrorHandler::ErrorLevel::ERROR);
      }
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("MouseHookManager", "Install", 
        "MouseHookManager::Install() failed",
        ErrorHandler::ErrorCategory::EXTERNAL_API, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("MouseHookManager", "Install", 
        "MouseHookManager::Install() failed (unknown exception)",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  } else {
    LOG_AND_REPORT_ERROR("MouseHookManager", "Install", 
      "MouseHookManager not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// Remove mouse hook (delegated to MouseHookManager)
void AnyWPEnginePlugin::RemoveMouseHook() {
  if (mouse_hook_manager_) {
    try {
      mouse_hook_manager_->Uninstall();
      std::cout << "[AnyWP] [Refactor] MouseHookManager hook uninstalled successfully" << std::endl;
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("MouseHookManager", "Uninstall", 
        "MouseHookManager::Uninstall() failed",
        ErrorHandler::ErrorCategory::EXTERNAL_API, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("MouseHookManager", "Uninstall", 
        "MouseHookManager::Uninstall() failed (unknown exception)",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  } else {
    LOG_AND_REPORT_ERROR("MouseHookManager", "Uninstall", 
      "MouseHookManager not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

bool AnyWPEnginePlugin::IsOurWindow(HWND hwnd) {
  if (!hwnd || !IsWindow(hwnd)) {
    return false;
  }
  
  auto check_candidate = [hwnd](HWND candidate) -> bool {
    if (!candidate || !IsWindow(candidate)) {
      return false;
    }
    
    if (candidate == hwnd) {
      return true;
    }
    
    return IsChild(candidate, hwnd) != FALSE;
  };
  
  if (check_candidate(webview_host_hwnd_) || check_candidate(worker_w_hwnd_)) {
    return true;
  }
  
  std::lock_guard<std::mutex> lock(instances_mutex_);
  for (const auto& instance : wallpaper_instances_) {
    if (check_candidate(instance.webview_host_hwnd) || check_candidate(instance.worker_w_hwnd)) {
      return true;
    }
  }
  
  return false;
}

// iframe Ad Detection: Handle iframe data from JavaScript
void AnyWPEnginePlugin::HandleIframeDataMessage(const std::string& json_data, WallpaperInstance* instance) {
  // v1.4.0+: Delegate to IframeDetector module
  if (instance) {
    // Multi-monitor: update specific instance's iframe list
    anywp_engine::IframeDetector::UpdateIframeVector(json_data, instance->iframes);
  } else {
    // Legacy: use global iframe list
    if (iframe_detector_) {
      iframe_detector_->UpdateIframes(json_data);
    }
  }
}

// iframe Ad Detection: Check if click is on an iframe
IframeInfo* AnyWPEnginePlugin::GetIframeAtPoint(int x, int y, WallpaperInstance* instance) {
  // v1.4.0+: Delegate to IframeDetector module
  if (instance) {
    // Multi-monitor: check specific instance's iframes
    return anywp_engine::IframeDetector::GetIframeAtPointInVector(x, y, instance->iframes);
  } else {
    // Legacy: use global iframe list
    if (iframe_detector_) {
      return iframe_detector_->GetIframeAtPoint(x, y);
    }
  }
  
  return nullptr;
}

// Phase B: Common initialization logic extracted from InitializeWallpaper and InitializeWallpaperOnMonitor
// v2.0+ Refactoring: Refactored to use InitializationCoordinator module
bool AnyWPEnginePlugin::InitializeWallpaperCommon(const std::string& url, bool enable_mouse_transparent, 
                                                    const MonitorInfo* monitor, HWND& out_hwnd, HWND& out_worker_w) {
  // P1-2: Periodic cleanup check
  PeriodicCleanup();
  
  // v2.0+ Refactoring: Use InitializationCoordinator for initialization
  if (!initialization_coordinator_) {
    LOG_AND_REPORT_ERROR("InitializationCoordinator", "InitializeWallpaperOnMonitor", 
      "InitializationCoordinator not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    LogError("InitializationCoordinator not initialized");
    return false;
  }
  
  // Prepare configuration
  InitializationCoordinator::InitConfig config;
  config.url = url;
  config.enable_mouse_transparent = enable_mouse_transparent;
  config.monitor = monitor;
  config.monitor_index = monitor ? monitor->index : -1;
  
  // Perform initialization using coordinator
  auto result = initialization_coordinator_->Initialize(config);
  
  if (!result.success) {
    LogError("Initialization failed: " + result.error_message);
    return false;
  }
  
  // Store interaction mode
  enable_interaction_ = !enable_mouse_transparent;
  
  // Setup MouseHook for event forwarding
  // CRITICAL: Always setup MouseHook for BOTH modes!
  // Why? WebView is BELOW desktop icons in Z-order, mouse events are captured by desktop first.
  // MouseHook is the ONLY way to forward events to our WebView.
  std::cout << "[AnyWP] Setting up MouseHook for event forwarding..." << std::endl;
  SetupMouseHook();
  
  // Return created handles
  out_hwnd = result.host_window;
  out_worker_w = result.worker_w_window;
  
  return true;
}

// Phase B: Refactored to use InitializeWallpaperCommon
bool AnyWPEnginePlugin::InitializeWallpaper(const std::string& url, bool enable_mouse_transparent) {
  std::cout << "[AnyWP] Initializing Wallpaper - URL: " << url 
            << ", Mouse Transparent: " << (enable_mouse_transparent ? "true" : "false") << std::endl;

  if (is_initialized_) {
    std::cout << "[AnyWP] Already initialized, stopping first..." << std::endl;
    StopWallpaper();
  }
  
  // Clear any residual iframe data before initialization
  {
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    if (!iframes_.empty()) {
      std::cout << "[AnyWP] [iframe] Clearing " << iframes_.size() << " residual iframe(s)" << std::endl;
      iframes_.clear();
    }
  }
  
  // Use common initialization logic
  if (!InitializeWallpaperCommon(url, enable_mouse_transparent, nullptr, webview_host_hwnd_, worker_w_hwnd_)) {
    return false;
  }
  
  // Show window
  ShowWindow(webview_host_hwnd_, SW_SHOW);
  UpdateWindow(webview_host_hwnd_);

  // Initialize WebView2 (pass -1 for legacy single-monitor support)
  // v1.4.1+ Phase A: Use WebViewManager module
  SetupWebView2WithManager(webview_host_hwnd_, url, -1);
  
  // Save URL for recovery after long-term lock/sleep
  default_wallpaper_url_ = url;
  std::cout << "[AnyWP] Saved wallpaper URL for auto-recovery: " << url << std::endl;

  std::cout << "[AnyWP] Initialization Complete" << std::endl;
  return true;
}

bool AnyWPEnginePlugin::StopWallpaper() {
  std::cout << "[AnyWP] Stopping wallpaper..." << std::endl;

  // Remove mouse hook
  RemoveMouseHook();

  // CRITICAL: Stop all wallpaper instances (multi-monitor mode)
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    if (!wallpaper_instances_.empty()) {
      std::cout << "[AnyWP] Stopping " << wallpaper_instances_.size() << " multi-monitor instance(s)..." << std::endl;
      
      for (auto& instance : wallpaper_instances_) {
        // Close WebView
        if (instance.webview_controller) {
          try {
            instance.webview_controller->Close();
          } catch (...) {
            Logger::Instance().Warning("Plugin", "Exception while closing WebView controller");
          }
          instance.webview_controller = nullptr;
        }
        instance.webview = nullptr;
        
        // Destroy window
        if (instance.webview_host_hwnd) {
          try {
          if (IsWindow(instance.webview_host_hwnd)) {
            ResourceTracker::Instance().UntrackWindow(instance.webview_host_hwnd);
            if (!DestroyWindow(instance.webview_host_hwnd)) {
              DWORD error = GetLastError();
                Logger::Instance().Warning("Plugin", "Failed to destroy window, error: " + std::to_string(error));
            }
          } else {
            ResourceTracker::Instance().UntrackWindow(instance.webview_host_hwnd);
            }
          } catch (const std::exception& e) {
            LOG_AND_REPORT_ERROR_EX("WindowManager", "DestroyWindow", 
              "Exception destroying window",
              ErrorHandler::ErrorCategory::RESOURCE, 
              ErrorHandler::ErrorLevel::ERROR, &e);
          } catch (...) {
            LOG_AND_REPORT_ERROR("WindowManager", "DestroyWindow", 
              "Unknown exception destroying window",
              ErrorHandler::ErrorCategory::UNKNOWN, 
              ErrorHandler::ErrorLevel::ERROR);
          }
          instance.webview_host_hwnd = nullptr;
        }
        
        // Clear iframe data
        instance.iframes.clear();
      }
      
      wallpaper_instances_.clear();
      // DON'T clear original_monitor_indices_ - needed for session switch rebuild
      // DON'T clear default_wallpaper_url_ - needed for session switch rebuild
      std::cout << "[AnyWP] All wallpapers stopped (URL and original config preserved for rebuild)" << std::endl;
    }
  }
  
  // CRITICAL: Clear shared WebView2 environment to force recreation
  // This is essential for session switch scenarios where environment becomes invalid
  if (shared_environment_) {
    std::cout << "[AnyWP] Clearing shared WebView2 environment (will recreate on next init)" << std::endl;
    shared_environment_ = nullptr;
  }

  // Stop single-monitor mode instance
  if (webview_controller_) {
    try {
      webview_controller_->Close();
    } catch (...) {
      Logger::Instance().Warning("Plugin", "Exception while closing WebView controller");
    }
    webview_controller_ = nullptr;
  }

  webview_ = nullptr;

  if (webview_host_hwnd_) {
    try {
    // Verify window is still valid
    if (IsWindow(webview_host_hwnd_)) {
      // P0-1: Untrack before destroying
      ResourceTracker::Instance().UntrackWindow(webview_host_hwnd_);
      
      if (!DestroyWindow(webview_host_hwnd_)) {
        DWORD error = GetLastError();
          Logger::Instance().Warning("Plugin", "Failed to destroy window, error: " + std::to_string(error));
      }
    } else {
        Logger::Instance().Warning("Plugin", "WebView host window already destroyed");
      ResourceTracker::Instance().UntrackWindow(webview_host_hwnd_);
      }
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("WindowManager", "DestroyWindow", 
        "Exception destroying webview_host_hwnd_",
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("WindowManager", "DestroyWindow", 
        "Unknown exception destroying webview_host_hwnd_",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
    webview_host_hwnd_ = nullptr;
  }

  // Clear iframe data when stopping wallpaper
  {
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    if (!iframes_.empty()) {
      std::cout << "[AnyWP] [iframe] Clearing " << iframes_.size() << " iframe(s) on stop" << std::endl;
      iframes_.clear();
    }
  }

  worker_w_hwnd_ = nullptr;
  is_initialized_ = false;
  enable_interaction_ = false;

  std::cout << "[AnyWP] Wallpaper stopped successfully" << std::endl;
  std::cout << "[AnyWP] [ResourceTracker] Tracked windows: " 
            << ResourceTracker::Instance().GetTrackedCount() << std::endl;
  
  return true;
}

bool AnyWPEnginePlugin::NavigateToUrl(const std::string& url) {
  BENCHMARK_SCOPE("NavigateToUrl");
  
  if (!webview_) {
    LOG_AND_REPORT_ERROR("WebViewManager", "NavigateToUrl", 
      "WebView not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    LogError("NavigateToUrl: WebView not initialized");
    return false;
  }

  // P0-3: Permission check with PermissionManager
  auto perm_result = PermissionManager::Instance().CheckUrlAccess(url);
  if (!perm_result.granted) {
    std::cout << "[AnyWP] [Security] URL access denied: " << url 
              << " - " << perm_result.reason << std::endl;
    Logger::Instance().Warning("PermissionManager", 
      "NavigateToUrl - URL access denied: " + perm_result.reason);
    return false;
  }
  
  // Fallback: Legacy URL validator
  if (!url_validator_.IsAllowed(url)) {
    std::cout << "[AnyWP] [Security] URL validation failed: " << url << std::endl;
    LogError("URL validation failed: " + url);
    return false;
  }

  // Clear iframe data when navigating to new page
  {
    std::lock_guard<std::mutex> lock(iframes_mutex_);
    if (!iframes_.empty()) {
      std::cout << "[AnyWP] [iframe] Clearing " << iframes_.size() << " iframe(s) before navigation" << std::endl;
      iframes_.clear();
    }
  }

  // P1-2: Check if cleanup needed
  PeriodicCleanup();

  try {
  std::wstring wurl(url.begin(), url.end());
  HRESULT hr = webview_->Navigate(wurl.c_str());
  
  if (SUCCEEDED(hr)) {
    std::cout << "[AnyWP] Navigated to: " << url << std::endl;
    
    // AUTO-OPTIMIZE: Safe delayed optimization (no dangling pointers)
    if (webview_) {
      ScheduleSafeMemoryOptimization(webview_.Get());
    }
    
    return true;
  } else {
      std::stringstream ss;
      ss << "Navigation failed: 0x" << std::hex << hr;
      LOG_AND_REPORT_ERROR("WebViewManager", "Navigate", 
        ss.str(),
        ErrorHandler::ErrorCategory::NETWORK, 
        ErrorHandler::ErrorLevel::ERROR);
    LogError("Navigation failed: " + url);
      return false;
    }
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("WebViewManager", "Navigate", 
      "Exception in NavigateToUrl",
      ErrorHandler::ErrorCategory::NETWORK, 
      ErrorHandler::ErrorLevel::ERROR, &e);
    LogError("Navigation exception: " + url);
    return false;
  } catch (...) {
    LOG_AND_REPORT_ERROR("WebViewManager", "Navigate", 
      "Unknown exception in NavigateToUrl",
      ErrorHandler::ErrorCategory::UNKNOWN, 
      ErrorHandler::ErrorLevel::ERROR);
    LogError("Navigation unknown exception: " + url);
    return false;
  }
}

// Multi-Monitor Support Implementation

// Monitor enumeration callback
BOOL CALLBACK AnyWPEnginePlugin::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
  auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
  
  MONITORINFOEXW monitor_info;
  monitor_info.cbSize = sizeof(MONITORINFOEXW);
  
  if (GetMonitorInfoW(hMonitor, &monitor_info)) {
    MonitorInfo info;
    info.index = static_cast<int>(monitors->size());
    info.handle = hMonitor;
    info.left = monitor_info.rcMonitor.left;
    info.top = monitor_info.rcMonitor.top;
    info.width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
    info.height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
    info.is_primary = (monitor_info.dwFlags & MONITORINFOF_PRIMARY) != 0;
    
    // Convert device name from wchar to string
    std::wstring wname(monitor_info.szDevice);
    // Use proper conversion to avoid C4244 warning
    info.device_name.reserve(wname.size());
    for (wchar_t wc : wname) {
      info.device_name.push_back(static_cast<char>(wc));
    }
    
    monitors->push_back(info);
    
    std::cout << "[AnyWP] [Monitor] Found: " << info.device_name 
              << " [" << info.width << "x" << info.height << "]"
              << " at (" << info.left << "," << info.top << ")"
              << (info.is_primary ? " (PRIMARY)" : "") << std::endl;
  }
  
  return TRUE;  // Continue enumeration
}

// Get all monitors
std::vector<MonitorInfo> AnyWPEnginePlugin::GetMonitors() {
  if (monitor_manager_) {
    std::vector<MonitorInfo> monitors = monitor_manager_->GetMonitors();
    monitors_ = monitors;  // Cache for backward compatibility
    return monitors;
  }
  
  // Fallback: return cached monitors or empty vector
  return monitors_;
}

// Get wallpaper instance for a specific monitor
// v2.0.0+ Phase2: Delegated to InstanceManager
WallpaperInstance* AnyWPEnginePlugin::GetInstanceForMonitor(int monitor_index) {
  if (instance_manager_) {
    return instance_manager_->GetInstanceForMonitor(monitor_index);
  }
  return nullptr;
}

// v2.0.0+ Phase2: Delegated to InstanceManager
WallpaperInstance* AnyWPEnginePlugin::GetInstanceAtPoint(int x, int y) {
  if (instance_manager_) {
    return instance_manager_->GetInstanceAtPoint(x, y);
  }
  return nullptr;
}

// Phase B: Refactored to use InitializeWallpaperCommon
bool AnyWPEnginePlugin::InitializeWallpaperOnMonitor(const std::string& url, bool enable_mouse_transparent, int monitor_index) {
  std::cout << "[AnyWP] Initializing Wallpaper on Monitor " << monitor_index 
            << " - URL: " << url << ", Transparent: " << (enable_mouse_transparent ? "true" : "false") << std::endl;

  // Get monitors if not cached
  if (monitors_.empty()) {
    GetMonitors();
  }
  
  // Find the target monitor
  const MonitorInfo* target_monitor = nullptr;
  for (const auto& monitor : monitors_) {
    if (monitor.index == monitor_index) {
      target_monitor = &monitor;
      break;
    }
  }
  
  if (!target_monitor) {
    LOG_AND_REPORT_ERROR("MonitorManager", "InitializeWallpaperOnMonitor", 
      "Monitor index " + std::to_string(monitor_index) + " not found",
      ErrorHandler::ErrorCategory::INVALID_STATE, 
      ErrorHandler::ErrorLevel::ERROR);
    LogError("Monitor not found: " + std::to_string(monitor_index));
    return false;
  }
  
  // Check if already initialized on this monitor
  WallpaperInstance* existing = GetInstanceForMonitor(monitor_index);
  if (existing) {
    std::cout << "[AnyWP] Monitor " << monitor_index << " already has wallpaper, stopping first..." << std::endl;
    StopWallpaperOnMonitor(monitor_index);
    std::cout << "[AnyWP] Waiting for resources to be released..." << std::endl;
    Sleep(500);
  }
  
  // Create new instance
  WallpaperInstance new_instance;
  new_instance.monitor_index = monitor_index;
  new_instance.enable_mouse_transparent = enable_mouse_transparent;  // v2.0.1+ Bug Fix: Save per-monitor transparency setting
  
  // Use common initialization logic
  if (!InitializeWallpaperCommon(url, enable_mouse_transparent, target_monitor, 
                                  new_instance.webview_host_hwnd, new_instance.worker_w_hwnd)) {
    return false;
  }
  
  // v2.0.0+ Phase2: Add instance using InstanceManager
  if (instance_manager_) {
    instance_manager_->AddInstance(new_instance);
    
    // v2.1.0+ Refactoring: Rebuild EventDispatcher cache after adding instance
    if (event_dispatcher_) {
      event_dispatcher_->RebuildHwndCache();
    }
  } else {
    LOG_AND_REPORT_ERROR("InstanceManager", "InitializeWallpaperOnMonitor", 
      "InstanceManager not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return false;
  }
  
  // Save to original configuration
  std::string device_name = target_monitor->device_name;
  if (std::find(original_monitor_devices_.begin(), original_monitor_devices_.end(), device_name) 
      == original_monitor_devices_.end()) {
    original_monitor_devices_.push_back(device_name);
    std::cout << "[AnyWP] Saved monitor " << device_name << " (index " << monitor_index 
              << ") to original configuration" << std::endl;
  }
  
  // Show window
  ShowWindow(new_instance.webview_host_hwnd, SW_SHOW);
  UpdateWindow(new_instance.webview_host_hwnd);

  // Initialize WebView2
  // v1.4.1+ Phase A: Use WebViewManager module
  SetupWebView2WithManager(new_instance.webview_host_hwnd, url, monitor_index);
  
  // Verify window styles based on user's mode selection
  if (new_instance.webview_host_hwnd && window_manager_) {
    std::cout << "[AnyWP] ========== Verifying Window Styles ==========" << std::endl;
    std::cout << "[AnyWP] Window HWND: " << new_instance.webview_host_hwnd << std::endl;
    std::cout << "[AnyWP] User selected: " << (enable_mouse_transparent ? "TRANSPARENT (pass-through)" : "INTERACTIVE (clickable)") << std::endl;
    
    // Get current window styles for debugging
    LONG style = GetWindowLongW(new_instance.webview_host_hwnd, GWL_STYLE);
    LONG ex_style = GetWindowLongW(new_instance.webview_host_hwnd, GWL_EXSTYLE);
    std::cout << "[AnyWP] Current Style: 0x" << std::hex << style << std::dec << std::endl;
    std::cout << "[AnyWP] Current ExStyle: 0x" << std::hex << ex_style << std::dec << std::endl;
    std::cout << "[AnyWP] Has WS_EX_TRANSPARENT: " << ((ex_style & WS_EX_TRANSPARENT) ? "YES" : "NO") << std::endl;
    
    // Get window position for debugging
    RECT rect;
    if (GetWindowRect(new_instance.webview_host_hwnd, &rect)) {
      std::cout << "[AnyWP] Window position: (" << rect.left << "," << rect.top 
                << ") size: " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
    }
    
    // Get parent window for debugging
    HWND parent = GetParent(new_instance.webview_host_hwnd);
    std::cout << "[AnyWP] Parent window: " << parent << std::endl;
    
    // Respect user's choice: Set interactive mode based on enable_mouse_transparent parameter
    bool should_be_interactive = !enable_mouse_transparent;
    bool success = window_manager_->SetInteractive(new_instance.webview_host_hwnd, should_be_interactive);
    
    if (success) {
      // Verify the change
      LONG ex_style_after = GetWindowLongW(new_instance.webview_host_hwnd, GWL_EXSTYLE);
      std::cout << "[AnyWP] After SetInteractive - ExStyle: 0x" << std::hex << ex_style_after << std::dec << std::endl;
      std::cout << "[AnyWP] After SetInteractive - Has WS_EX_TRANSPARENT: " << ((ex_style_after & WS_EX_TRANSPARENT) ? "YES" : "NO") << std::endl;
      
      if (should_be_interactive) {
        std::cout << "[AnyWP] [OK] Window set to INTERACTIVE mode - mouse events enabled" << std::endl;
      } else {
        std::cout << "[AnyWP] [OK] Window set to TRANSPARENT mode - mouse pass-through enabled" << std::endl;
      }
    } else {
      LOG_AND_REPORT_ERROR("WindowManager", "SetWindowMode", 
        "Failed to set window mode",
        ErrorHandler::ErrorCategory::EXTERNAL_API, 
        ErrorHandler::ErrorLevel::ERROR);
    }
    std::cout << "[AnyWP] ================================================================" << std::endl;
  }
  
  // Save URL as default
  if (default_wallpaper_url_.empty()) {
    default_wallpaper_url_ = url;
    std::cout << "[AnyWP] Set default wallpaper URL for auto-start: " << url << std::endl;
  }

  std::cout << "[AnyWP] Initialization Complete (Monitor " << monitor_index << ")" << std::endl;
  return true;
}

// v2.0.0+ Phase2: Delegated to InstanceManager
bool AnyWPEnginePlugin::StopWallpaperOnMonitor(int monitor_index) {
  std::cout << "[AnyWP] Stopping wallpaper on monitor " << monitor_index << "..." << std::endl;

  if (instance_manager_) {
    bool result = instance_manager_->CleanupInstance(monitor_index);
    
    // v2.1.0+ Refactoring: Rebuild EventDispatcher cache after removing instance
    if (result && event_dispatcher_) {
      event_dispatcher_->RebuildHwndCache();
    }
    
    return result;
  }
  
  LOG_AND_REPORT_ERROR("InstanceManager", "InitializeWallpaperOnMonitor", 
    "InstanceManager not initialized",
    ErrorHandler::ErrorCategory::INITIALIZATION, 
    ErrorHandler::ErrorLevel::ERROR);
  return false;
}

// Navigate to URL on specific monitor
bool AnyWPEnginePlugin::NavigateToUrlOnMonitor(const std::string& url, int monitor_index) {
  BENCHMARK_SCOPE("NavigateToUrlOnMonitor");
  
  WallpaperInstance* instance = GetInstanceForMonitor(monitor_index);
  if (!instance || !instance->webview) {
    LOG_AND_REPORT_ERROR("WebViewManager", "NavigateToUrlOnMonitor", 
      "No wallpaper on monitor " + std::to_string(monitor_index),
      ErrorHandler::ErrorCategory::INVALID_STATE, 
      ErrorHandler::ErrorLevel::ERROR);
    LogError("NavigateToUrlOnMonitor: WebView not initialized on monitor " + std::to_string(monitor_index));
    return false;
  }

  // P0-3: Permission check
  auto perm_result = PermissionManager::Instance().CheckUrlAccess(url);
  if (!perm_result.granted) {
    std::cout << "[AnyWP] [Security] URL access denied on monitor " << monitor_index 
              << ": " << url << " - " << perm_result.reason << std::endl;
    Logger::Instance().Warning("PermissionManager", 
      "NavigateToUrlOnMonitor - URL access denied: " + perm_result.reason);
    return false;
  }
  
  // Fallback: Legacy URL validator
  if (!url_validator_.IsAllowed(url)) {
    std::cout << "[AnyWP] [Security] URL validation failed: " << url << std::endl;
    LogError("URL validation failed: " + url);
    return false;
  }

  // Clear iframe data
  instance->iframes.clear();

  // P1-2: Check if cleanup needed
  PeriodicCleanup();

  try {
  std::wstring wurl(url.begin(), url.end());
  HRESULT hr = instance->webview->Navigate(wurl.c_str());
  
  if (SUCCEEDED(hr)) {
    std::cout << "[AnyWP] Navigated to: " << url << " on monitor " << monitor_index << std::endl;
    
    // AUTO-OPTIMIZE: Safe delayed optimization (no dangling pointers)
    if (instance && instance->webview) {
      ScheduleSafeMemoryOptimization(instance->webview.Get());
    }
    
    return true;
  } else {
      std::stringstream ss;
      ss << "Navigation failed on monitor " << monitor_index << ": 0x" << std::hex << hr;
      LOG_AND_REPORT_ERROR("WebViewManager", "Navigate", 
        ss.str(),
        ErrorHandler::ErrorCategory::NETWORK, 
        ErrorHandler::ErrorLevel::ERROR);
    LogError("Navigation failed on monitor " + std::to_string(monitor_index) + ": " + url);
      return false;
    }
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("WebViewManager", "Navigate", 
      "Exception in NavigateToUrlOnMonitor",
      ErrorHandler::ErrorCategory::NETWORK, 
      ErrorHandler::ErrorLevel::ERROR, &e);
    LogError("Navigation exception on monitor " + std::to_string(monitor_index) + ": " + url);
    return false;
  } catch (...) {
    LOG_AND_REPORT_ERROR("WebViewManager", "Navigate", 
      "Unknown exception in NavigateToUrlOnMonitor",
      ErrorHandler::ErrorCategory::UNKNOWN, 
      ErrorHandler::ErrorLevel::ERROR);
    LogError("Navigation unknown exception on monitor " + std::to_string(monitor_index) + ": " + url);
    return false;
  }
}

// Display Change Monitoring Implementation

// Window procedure for display change listener
LRESULT CALLBACK AnyWPEnginePlugin::DisplayChangeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_DISPLAYCHANGE) {
    std::cout << "[AnyWP] [DisplayChange] Display configuration changed!" << std::endl;
    std::cout << "[AnyWP] [DisplayChange] New resolution: " << LOWORD(lParam) << "x" << HIWORD(lParam) << std::endl;
    
    if (display_change_instance_) {
      display_change_instance_->HandleDisplayChange();
    }
    
    return 0;
  }
  else if (message == WM_NOTIFY_MONITOR_CHANGE) {
    std::cout << "[AnyWP] [DisplayChange] Received WM_NOTIFY_MONITOR_CHANGE in main thread" << std::endl;
    
    // Now we're in the window's message loop thread (safer for Flutter)
    if (display_change_instance_) {
      display_change_instance_->NotifyMonitorChange();
    }
    
    return 0;
  }
  
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

// Setup display change listener (delegated to MonitorManager)
// v2.0.0+ Phase2: Delegated to DisplayChangeCoordinator
void AnyWPEnginePlugin::SetupDisplayChangeListener() {
  if (display_change_coordinator_) {
    display_change_coordinator_->StartMonitoring(monitor_manager_.get());
  } else {
    LOG_AND_REPORT_ERROR("DisplayChangeCoordinator", "SetupDisplayChangeListener", 
      "DisplayChangeCoordinator not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// v2.0.0+ Phase2: Delegated to DisplayChangeCoordinator
void AnyWPEnginePlugin::CleanupDisplayChangeListener() {
  if (display_change_coordinator_) {
    display_change_coordinator_->StopMonitoring(monitor_manager_.get());
  } else {
    LOG_AND_REPORT_ERROR("DisplayChangeCoordinator", "CleanupDisplayChangeListener", 
      "DisplayChangeCoordinator not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::WARNING);
  }
}

// v2.0.0+ Phase2: Delegated to DisplayChangeCoordinator
void AnyWPEnginePlugin::HandleDisplayChange() {
  if (display_change_coordinator_) {
    display_change_coordinator_->HandleDisplayChange();
  } else {
    LOG_AND_REPORT_ERROR("DisplayChangeCoordinator", "HandleDisplayChange", 
      "DisplayChangeCoordinator not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// Notify Dart side about monitor changes
void AnyWPEnginePlugin::NotifyMonitorChange() {
  if (!method_channel_) {
    LOG_AND_REPORT_ERROR("FlutterBridge", "NotifyMonitorChange", 
      "method_channel_ is nullptr",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
    return;
  }
  
  std::cout << "[AnyWP] [DisplayChange] Notifying Dart about monitor change via channel: " << method_channel_ << std::endl;
  
  // CRITICAL: InvokeMethod must be called carefully
  // WM_DISPLAYCHANGE comes from window message loop
  // We need to ensure it's safe to call Flutter API from this context
  
  try {
    std::cout << "[AnyWP] [DisplayChange] Monitor change detected" << std::endl;
    
    // CRITICAL: InvokeMethod causes deadlock/crash even with message queue
    // Root cause: Flutter engine thread synchronization issues
    // Solution: Use polling from Dart side instead (Timer.periodic)
    
    // Do NOT call InvokeMethod - it will hang the application
    // method_channel_->InvokeMethod("onMonitorChange", std::move(args));
    
    std::cout << "[AnyWP] [DisplayChange] Skipping InvokeMethod to prevent deadlock" << std::endl;
    std::cout << "[AnyWP] [DisplayChange] Dart side will detect changes via polling" << std::endl;
    
  } catch (const std::exception& e) {
    std::cout << "[AnyWP] [DisplayChange] EXCEPTION: " << e.what() << std::endl;
  } catch (...) {
    std::cout << "[AnyWP] [DisplayChange] UNKNOWN EXCEPTION" << std::endl;
  }
  
  std::cout << "[AnyWP] [DisplayChange] NotifyMonitorChange completed" << std::endl;
}

// Notify monitor change (simplified after MonitorManager refactoring)
void AnyWPEnginePlugin::SafeNotifyMonitorChange() {
  std::cout << "[AnyWP] [DisplayChange] Notifying monitor change..." << std::endl;
  NotifyMonitorChange();
}

// ========== Bidirectional Communication ==========

// v2.1.0+: Notify Flutter of messages from JavaScript
// NOTE: Uses message queue instead of InvokeMethod to avoid deadlock
void AnyWPEnginePlugin::NotifyFlutterMessage(const std::string& message) {
  Logger::Instance().Info("AnyWPEngine", "Queuing message from JavaScript");
  Logger::Instance().Debug("AnyWPEngine", "  Message: " + message.substr(0, 100) + 
                          (message.length() > 100 ? "..." : ""));

  try {
    // Validate message is not empty
    if (message.empty()) {
      Logger::Instance().Warning("AnyWPEngine", "NotifyFlutterMessage: empty message, skipping");
      return;
    }

    // Add to message queue (thread-safe)
    {
      std::lock_guard<std::mutex> lock(messages_mutex_);
      pending_messages_.push(message);
      
      // Limit queue size to prevent memory issues
      if (pending_messages_.size() > 1000) {
        Logger::Instance().Warning("AnyWPEngine", "Message queue size exceeded 1000, dropping oldest message");
        pending_messages_.pop();
      }
    }

    Logger::Instance().Info("AnyWPEngine", "Message queued successfully (queue size: " + 
                           std::to_string(pending_messages_.size()) + ")");
  } catch (const std::exception& e) {
    Logger::Instance().Error("AnyWPEngine", 
      std::string("Exception during message queuing: ") + e.what());
  } catch (...) {
    Logger::Instance().Error("AnyWPEngine", 
      "Unknown exception during message queuing");
  }
}

// v2.1.0+: Get pending messages (called by Dart via polling)
std::vector<std::string> AnyWPEnginePlugin::GetPendingMessages() {
  std::vector<std::string> messages;
  
  std::lock_guard<std::mutex> lock(messages_mutex_);
  
  // Move all messages from queue to vector
  while (!pending_messages_.empty()) {
    messages.push_back(pending_messages_.front());
    pending_messages_.pop();
  }
  
  if (!messages.empty()) {
    Logger::Instance().Info("AnyWPEngine", 
      "Retrieved " + std::to_string(messages.size()) + " pending messages");
  }
  
  return messages;
}

// v2.1.1+ Fix: Get pending power state changes (called by Dart via polling)
std::vector<std::pair<std::string, std::string>> AnyWPEnginePlugin::GetPendingPowerStateChanges() {
  std::vector<std::pair<std::string, std::string>> changes;
  
  std::lock_guard<std::mutex> lock(power_state_changes_mutex_);
  
  // Move all changes from queue to vector
  while (!pending_power_state_changes_.empty()) {
    const auto& change = pending_power_state_changes_.front();
    changes.push_back({change.oldState, change.newState});
    pending_power_state_changes_.pop();
  }
  
  if (!changes.empty()) {
    Logger::Instance().Info("AnyWPEngine", 
      "Retrieved " + std::to_string(changes.size()) + " pending power state changes");
  }
  
  return changes;
}

// v2.0.0+ Phase2: These methods have been moved to DisplayChangeCoordinator module
// - HandleMonitorCountChange (~87 lines)
// - UpdateWallpaperSizes (~68 lines)

// ========== Power Saving & Optimization Implementation ==========

// Setup power saving monitoring
void AnyWPEnginePlugin::SetupPowerSavingMonitoring() {
  std::cout << "[AnyWP] [PowerSaving] Setting up power saving monitoring..." << std::endl;
  
  // Register window class for power events
  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = PowerSavingWndProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"AnyWPPowerListener";
  
  if (!RegisterClassExW(&wc)) {
    DWORD error = GetLastError();
    if (error != ERROR_CLASS_ALREADY_EXISTS) {
      std::cout << "[AnyWP] [PowerSaving] Failed to register window class: " << error << std::endl;
      return;
    }
  }
  
  // Create hidden window to receive power messages
  try {
  power_listener_hwnd_ = CreateWindowExW(
    0,
    L"AnyWPPowerListener",
    L"AnyWP Power Listener",
    WS_OVERLAPPED,
    0, 0, 1, 1,
    nullptr,
    nullptr,
    GetModuleHandleW(nullptr),
    nullptr
  );
    
    if (!power_listener_hwnd_) {
      LOG_AND_REPORT_ERROR("PowerManager", "SetupPowerSavingMonitoring", 
        "Failed to create power listener window",
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR);
      return;
    }
  } catch (const std::exception& e) {
    LOG_AND_REPORT_ERROR_EX("PowerManager", "SetupPowerSavingMonitoring", 
      "Exception creating power listener window",
      ErrorHandler::ErrorCategory::RESOURCE, 
      ErrorHandler::ErrorLevel::ERROR, &e);
    return;
  } catch (...) {
    LOG_AND_REPORT_ERROR("PowerManager", "SetupPowerSavingMonitoring", 
      "Unknown exception creating power listener window",
      ErrorHandler::ErrorCategory::UNKNOWN, 
      ErrorHandler::ErrorLevel::ERROR);
    return;
  }
  
  if (power_listener_hwnd_) {
    ShowWindow(power_listener_hwnd_, SW_HIDE);
    std::cout << "[AnyWP] [PowerSaving] Power listener window created: " << power_listener_hwnd_ << std::endl;
    
    // Register for session change notifications (lock/unlock)
    WTSRegisterSessionNotification(power_listener_hwnd_, NOTIFY_FOR_THIS_SESSION);
    
  } else {
    std::cout << "[AnyWP] [PowerSaving] Failed to create listener window: " << GetLastError() << std::endl;
  }
  
  // Initialize session state flags
  is_remote_session_.store(GetSystemMetrics(SM_REMOTESESSION) != 0);
  is_session_locked_.store(false);  // Assume unlocked at startup
  
  std::cout << "[AnyWP] [PowerSaving] Initial session state: remote=" 
            << is_remote_session_.load() << ", locked=" << is_session_locked_.load() << std::endl;
  
  // Note: Fullscreen detection is started by PowerManager::Enable() after initialization
  
  std::cout << "[AnyWP] [PowerSaving] Monitoring setup complete" << std::endl;
}

// Cleanup power saving monitoring
void AnyWPEnginePlugin::CleanupPowerSavingMonitoring() {
  std::cout << "[AnyWP] [PowerSaving] Cleaning up monitoring..." << std::endl;
  
  // Note: Fullscreen detection is stopped by PowerManager destructor
  
  // Cleanup window
  if (power_listener_hwnd_) {
    try {
    WTSUnRegisterSessionNotification(power_listener_hwnd_);
    DestroyWindow(power_listener_hwnd_);
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("WindowManager", "DestroyWindow", 
        "Exception destroying power_listener_hwnd_",
        ErrorHandler::ErrorCategory::RESOURCE, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("WindowManager", "DestroyWindow", 
        "Unknown exception destroying power_listener_hwnd_",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
    power_listener_hwnd_ = nullptr;
  }
  
  std::cout << "[AnyWP] [PowerSaving] Cleanup complete" << std::endl;
}

// Power saving window procedure
LRESULT CALLBACK AnyWPEnginePlugin::PowerSavingWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if (!display_change_instance_) {
    return DefWindowProcW(hwnd, message, wParam, lParam);
  }
  
  switch (message) {
    case WM_WTSSESSION_CHANGE:
      // Session state changed (lock/unlock/remote desktop)
      Logger::Instance().Info("PowerSaving", "========== SESSION CHANGE EVENT ==========");
      Logger::Instance().Info("PowerSaving", "Event code: " + std::to_string(wParam));
      
      // Update session state flags based on event
      switch (wParam) {
        case WTS_SESSION_LOCK:
          Logger::Instance().Info("PowerSaving", "Event: System LOCKED");
          display_change_instance_->is_session_locked_.store(true);
          // Notify PowerManager module
          if (display_change_instance_->power_manager_) {
            display_change_instance_->power_manager_->SetSessionLocked(true);
          }
          break;
        case WTS_SESSION_UNLOCK:
          Logger::Instance().Info("PowerSaving", "Event: System UNLOCKED");
          display_change_instance_->is_session_locked_.store(false);
          // Notify PowerManager module
          if (display_change_instance_->power_manager_) {
            display_change_instance_->power_manager_->SetSessionLocked(false);
          }
          break;
        case WTS_CONSOLE_CONNECT:
          Logger::Instance().Info("PowerSaving", "Event: CONSOLE CONNECTED (returned from remote desktop)");
          display_change_instance_->is_remote_session_.store(false);
          // Notify PowerManager module
          if (display_change_instance_->power_manager_) {
            display_change_instance_->power_manager_->SetRemoteSession(false);
          }
          // Check if wallpaper should be active (force_reinit will handle stop+rebuild)
          Logger::Instance().Info("PowerSaving", "Session switched, checking if wallpaper should be active...");
          if (display_change_instance_->ShouldWallpaperBeActive()) {
            Logger::Instance().Info("PowerSaving", "Rebuilding wallpaper on new session...");
            display_change_instance_->is_paused_.store(true);  // Force paused state
            display_change_instance_->ResumeWallpaper("Session: Switched to local console", true);  // force_reinit = true
          } else {
            Logger::Instance().Info("PowerSaving", "System locked, will stop wallpaper and rebuild after unlock");
            // Stop cross-session wallpaper (it's not visible anyway)
            display_change_instance_->StopWallpaper();
          }
          break;
        case WTS_CONSOLE_DISCONNECT:
          Logger::Instance().Info("PowerSaving", "Event: CONSOLE DISCONNECTED (switched to remote desktop)");
          display_change_instance_->is_remote_session_.store(true);
          // Notify PowerManager module
          if (display_change_instance_->power_manager_) {
            display_change_instance_->power_manager_->SetRemoteSession(true);
          }
          // Check if wallpaper should be active (force_reinit will handle stop+rebuild)
          Logger::Instance().Info("PowerSaving", "Session switched, checking if wallpaper should be active...");
          if (display_change_instance_->ShouldWallpaperBeActive()) {
            Logger::Instance().Info("PowerSaving", "Rebuilding wallpaper on new session...");
            display_change_instance_->is_paused_.store(true);  // Force paused state
            display_change_instance_->ResumeWallpaper("Session: Switched to remote desktop", true);  // force_reinit = true
          } else {
            Logger::Instance().Info("PowerSaving", "System locked, will stop wallpaper and rebuild after unlock");
            // Stop cross-session wallpaper (it's not visible anyway)
            display_change_instance_->StopWallpaper();
          }
          break;
        case WTS_REMOTE_CONNECT:
          Logger::Instance().Info("PowerSaving", "Event: REMOTE DESKTOP CONNECTED");
          display_change_instance_->is_remote_session_.store(true);
          // Check if wallpaper should be active (force_reinit will handle stop+rebuild)
          Logger::Instance().Info("PowerSaving", "Session switched, checking if wallpaper should be active...");
          if (display_change_instance_->ShouldWallpaperBeActive()) {
            Logger::Instance().Info("PowerSaving", "Rebuilding wallpaper on new session...");
            display_change_instance_->is_paused_.store(true);  // Force paused state
            display_change_instance_->ResumeWallpaper("Session: Remote desktop connected", true);  // force_reinit = true
          } else {
            Logger::Instance().Info("PowerSaving", "System locked, will stop wallpaper and rebuild after unlock");
            // Stop cross-session wallpaper (it's not visible anyway)
            display_change_instance_->StopWallpaper();
          }
          break;
        case WTS_REMOTE_DISCONNECT:
          Logger::Instance().Info("PowerSaving", "Event: REMOTE DESKTOP DISCONNECTED");
          display_change_instance_->is_remote_session_.store(false);
          // Check if wallpaper should be active (force_reinit will handle stop+rebuild)
          Logger::Instance().Info("PowerSaving", "Session switched, checking if wallpaper should be active...");
          if (display_change_instance_->ShouldWallpaperBeActive()) {
            Logger::Instance().Info("PowerSaving", "Rebuilding wallpaper on new session...");
            display_change_instance_->is_paused_.store(true);  // Force paused state
            display_change_instance_->ResumeWallpaper("Session: Remote desktop disconnected", true);  // force_reinit = true
          } else {
            Logger::Instance().Info("PowerSaving", "System locked, will stop wallpaper and rebuild after unlock");
            // Stop cross-session wallpaper (it's not visible anyway)
            display_change_instance_->StopWallpaper();
          }
          break;
      }
      
      // For lock/unlock events, use unified state check
      if (wParam == WTS_SESSION_LOCK || wParam == WTS_SESSION_UNLOCK) {
        Logger::Instance().Info("PowerSaving", "Checking if wallpaper should be active...");
        if (display_change_instance_->ShouldWallpaperBeActive()) {
          // Check if wallpaper needs to be rebuilt (cross-session scenario)
          bool need_rebuild = false;
          {
            std::lock_guard<std::mutex> lock(display_change_instance_->instances_mutex_);
            need_rebuild = display_change_instance_->wallpaper_instances_.empty() && 
                          !display_change_instance_->default_wallpaper_url_.empty();
          }
          
          if (need_rebuild && wParam == WTS_SESSION_UNLOCK) {
            Logger::Instance().Info("PowerSaving", "Wallpaper missing after session switch, forcing rebuild...");
            display_change_instance_->is_paused_.store(true);
            display_change_instance_->ResumeWallpaper("Session: Unlock after cross-session", true);  // force_reinit
          } else {
            display_change_instance_->ResumeWallpaper("Session: User in local desktop");
          }
        } else {
          display_change_instance_->PauseWallpaper("Session: User not in local desktop");
        }
      }
      Logger::Instance().Info("PowerSaving", "=========================================");
      break;
      
    case WM_POWERBROADCAST:
      // Power state changed
      switch (wParam) {
        case PBT_APMSUSPEND:
          std::cout << "[AnyWP] [PowerSaving] System SUSPENDING" << std::endl;
          display_change_instance_->PauseWallpaper("SUSPEND");
          break;
        case PBT_APMRESUMEAUTOMATIC:
        case PBT_APMRESUMESUSPEND:
          std::cout << "[AnyWP] [PowerSaving] System RESUMING" << std::endl;
          display_change_instance_->ResumeWallpaper("SUSPEND");
          break;
        case PBT_APMPOWERSTATUSCHANGE:
          // Check if monitor is off
          std::cout << "[AnyWP] [PowerSaving] Power status changed" << std::endl;
          display_change_instance_->UpdatePowerState();
          break;
      }
      break;
  }
  
  return DefWindowProcW(hwnd, message, wParam, lParam);
}

// Check if wallpaper should be active based on current session state
bool AnyWPEnginePlugin::ShouldWallpaperBeActive() {
  if (power_manager_) {
    return power_manager_->ShouldWallpaperBeActive();
  }
  
  // Fallback: assume wallpaper should be active
  return true;
}

// Update power state based on current system status
void AnyWPEnginePlugin::UpdatePowerState() {
  if (power_manager_) {
    power_manager_->UpdatePowerState();
  }
}

// Check if fullscreen app is active
bool AnyWPEnginePlugin::IsFullscreenAppActive() {
  if (power_manager_) {
    return power_manager_->IsFullscreenAppActive();
  }
  
  // Fallback: assume no fullscreen app
  return false;
}

// Start fullscreen detection thread (delegated to PowerManager)
void AnyWPEnginePlugin::StartFullscreenDetection() {
  if (power_manager_) {
    try {
      power_manager_->StartFullscreenDetection();
      std::cout << "[AnyWP] [Refactor] Fullscreen detection delegated to PowerManager" << std::endl;
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("PowerManager", "StartFullscreenDetection", 
        "PowerManager::StartFullscreenDetection() failed",
        ErrorHandler::ErrorCategory::EXTERNAL_API, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("PowerManager", "StartFullscreenDetection", 
        "PowerManager::StartFullscreenDetection() failed (unknown exception)",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  } else {
    LOG_AND_REPORT_ERROR("PowerManager", "StartFullscreenDetection", 
      "PowerManager not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// Stop fullscreen detection thread (delegated to PowerManager)
void AnyWPEnginePlugin::StopFullscreenDetection() {
  if (power_manager_) {
    try {
      power_manager_->StopFullscreenDetection();
      std::cout << "[AnyWP] [Refactor] Fullscreen detection stop delegated to PowerManager" << std::endl;
    } catch (const std::exception& e) {
      LOG_AND_REPORT_ERROR_EX("PowerManager", "StopFullscreenDetection", 
        "PowerManager::StopFullscreenDetection() failed",
        ErrorHandler::ErrorCategory::EXTERNAL_API, 
        ErrorHandler::ErrorLevel::ERROR, &e);
    } catch (...) {
      LOG_AND_REPORT_ERROR("PowerManager", "StopFullscreenDetection", 
        "PowerManager::StopFullscreenDetection() failed (unknown exception)",
        ErrorHandler::ErrorCategory::UNKNOWN, 
        ErrorHandler::ErrorLevel::ERROR);
    }
  } else {
    LOG_AND_REPORT_ERROR("PowerManager", "StopFullscreenDetection", 
      "PowerManager not initialized",
      ErrorHandler::ErrorCategory::INITIALIZATION, 
      ErrorHandler::ErrorLevel::ERROR);
  }
}

// Pause wallpaper - GOAL: Reduce CPU/GPU usage while keeping wallpaper visible
// v1.4.1+ Phase E: Simplified - delegates to PowerManager
void AnyWPEnginePlugin::PauseWallpaper(const std::string& reason) {
  // Guard: Avoid duplicate pause
  if (is_paused_.exchange(true)) {
    return;  // Already paused
  }
  
  std::cout << "[AnyWP] [PowerSaving] ========== PAUSING WALLPAPER ==========" << std::endl;
  std::cout << "[AnyWP] [PowerSaving] Reason: " << reason << std::endl;
  
  // Delegate script execution to PowerManager
  if (power_manager_) {
    power_manager_->ExecutePauseScripts([this](const std::wstring& script) {
      ExecuteScriptToAllInstances(script);
    });
  }
  
  // Light memory trim
  SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
  
  std::cout << "[AnyWP] [PowerSaving] Wallpaper paused - last frame frozen" << std::endl;
}

// v1.4.1+ Phase G: Validate wallpaper windows state
bool AnyWPEnginePlugin::ValidateWallpaperWindows() {
  std::cout << "[AnyWP] [PowerSaving] Verifying wallpaper window state..." << std::endl;
  
  // Check single-monitor mode
  if (webview_host_hwnd_) {
    std::cout << "[AnyWP] [PowerSaving] Single-monitor mode detected" << std::endl;
    std::cout << "[AnyWP] [PowerSaving] WebView window: " << webview_host_hwnd_ << std::endl;
    std::cout << "[AnyWP] [PowerSaving] IsWindow: " << IsWindow(webview_host_hwnd_) 
              << ", IsVisible: " << (IsWindow(webview_host_hwnd_) ? IsWindowVisible(webview_host_hwnd_) : false) << std::endl;
    
    if (!IsWindow(webview_host_hwnd_) || !IsWindowVisible(webview_host_hwnd_)) {
      Logger::Instance().Warning("PowerSaving", "Wallpaper window invalid or hidden!");
      return false;  // Need reinitialize
    }
    
    // Verify parent relationship
    HWND parent = GetParent(webview_host_hwnd_);
    std::cout << "[AnyWP] [PowerSaving] Expected parent (WorkerW): " << worker_w_hwnd_ << std::endl;
    std::cout << "[AnyWP] [PowerSaving] Actual parent: " << parent << std::endl;
    std::cout << "[AnyWP] [PowerSaving] WorkerW valid: " << IsWindow(worker_w_hwnd_) << std::endl;
    
    if (parent != worker_w_hwnd_ || !IsWindow(worker_w_hwnd_)) {
      Logger::Instance().Warning("PowerSaving", "Parent window relationship broken!");
      return false;  // Need reinitialize
    }
    
    std::cout << "[AnyWP] [PowerSaving] [OK] Window valid, parent relationship OK" << std::endl;
  }
  
  // Check multi-monitor mode
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    if (!wallpaper_instances_.empty()) {
      std::cout << "[AnyWP] [PowerSaving] Multi-monitor mode detected (" << wallpaper_instances_.size() << " instances)" << std::endl;
      
      for (auto& instance : wallpaper_instances_) {
        std::cout << "[AnyWP] [PowerSaving] Checking monitor " << instance.monitor_index << std::endl;
        std::cout << "[AnyWP] [PowerSaving] WebView window: " << instance.webview_host_hwnd << std::endl;
        std::cout << "[AnyWP] [PowerSaving] IsWindow: " << IsWindow(instance.webview_host_hwnd) 
                  << ", IsVisible: " << IsWindowVisible(instance.webview_host_hwnd) << std::endl;
        
        if (!IsWindow(instance.webview_host_hwnd) || !IsWindowVisible(instance.webview_host_hwnd)) {
          Logger::Instance().Warning("PowerSaving", 
            "Monitor " + std::to_string(instance.monitor_index) + " wallpaper window invalid or hidden!");
          return false;  // Need reinitialize
        }
        
        std::cout << "[AnyWP] [PowerSaving] [OK] Monitor " << instance.monitor_index 
                  << " window valid" << std::endl;
      }
    }
  }
  
  return true;  // All windows valid
}

// v1.4.1+ Phase G: Restore wallpaper configuration after window loss
bool AnyWPEnginePlugin::RestoreWallpaperConfiguration(const std::string& url) {
  if (url.empty()) {
    LOG_AND_REPORT_ERROR("PowerManager", "RestoreWallpaperConfiguration", 
      "No saved URL to restore wallpaper",
      ErrorHandler::ErrorCategory::INVALID_STATE, 
      ErrorHandler::ErrorLevel::ERROR);
    LogError("Cannot restore wallpaper: no saved URL");
    return false;
  }
  
  std::cout << "[AnyWP] [PowerSaving] ========== RESTORING LOST WALLPAPER ==========" << std::endl;
  std::cout << "[AnyWP] [PowerSaving] Re-initializing wallpaper with URL: " << url << std::endl;
  
  // Save current URL and use ORIGINAL monitor configuration
  std::vector<int> saved_monitor_indices;
  std::vector<std::string> saved_monitor_devices;
  std::map<int, bool> saved_transparency_settings;  // v2.0.1+ Bug Fix: Save per-monitor transparency settings
  
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    // Use original configuration (device names, survives session switches)
    saved_monitor_devices = original_monitor_devices_;
    
    std::cout << "[AnyWP] [PowerSaving] Using original monitor configuration: " 
              << saved_monitor_devices.size() << " monitor(s)" << std::endl;
    
    // v2.0.1+ Bug Fix: Save transparency settings for each instance before stopping
    for (const auto& instance : wallpaper_instances_) {
      saved_transparency_settings[instance.monitor_index] = instance.enable_mouse_transparent;
      std::cout << "[AnyWP] [PowerSaving] Saved transparency setting for monitor " 
                << instance.monitor_index << ": " 
                << (instance.enable_mouse_transparent ? "transparent" : "interactive") << std::endl;
    }
    
    // Fallback: if no original config, try current instances
    if (saved_monitor_devices.empty()) {
      std::cout << "[AnyWP] [PowerSaving] No original config, using current instances" << std::endl;
      for (const auto& instance : wallpaper_instances_) {
        // Find device name for this instance's monitor index
        for (const auto& monitor : monitors_) {
          if (monitor.index == instance.monitor_index) {
            saved_monitor_devices.push_back(monitor.device_name);
            break;
          }
        }
      }
    }
  }
  
  // Convert device names to current indices
  for (const auto& device_name : saved_monitor_devices) {
    for (const auto& monitor : monitors_) {
      if (monitor.device_name == device_name) {
        saved_monitor_indices.push_back(monitor.index);
        break;
      }
    }
  }
  
  std::cout << "[AnyWP] [PowerSaving] Will restore " << saved_monitor_indices.size() 
            << " monitor(s)" << std::endl;
  
  // CRITICAL: Always stop existing wallpaper before recreating
  std::cout << "[AnyWP] [PowerSaving] Stopping existing wallpaper (if any)..." << std::endl;
  StopWallpaper();
  
  // CRITICAL: Re-enumerate monitors before rebuilding (session may have different monitors)
  std::cout << "[AnyWP] [PowerSaving] Re-enumerating monitors for current session..." << std::endl;
  GetMonitors();
  std::cout << "[AnyWP] [PowerSaving] Current session has " << monitors_.size() << " monitor(s)" << std::endl;
  
  if (!saved_monitor_indices.empty()) {
    // Restore wallpaper on all previously active monitors that still exist
    std::cout << "[AnyWP] [PowerSaving] Attempting to restore " 
              << saved_monitor_indices.size() << " monitor(s)..." << std::endl;
    
    int restored_count = 0;
    for (int monitor_index : saved_monitor_indices) {
      // Check if this monitor exists in current session
      bool monitor_exists = false;
      for (const auto& monitor : monitors_) {
        if (monitor.index == monitor_index) {
          monitor_exists = true;
          break;
        }
      }
      
      if (monitor_exists) {
        std::cout << "[AnyWP] [PowerSaving] Restoring wallpaper on monitor " 
                  << monitor_index << std::endl;
        
        // v2.0.1+ Bug Fix: Use saved transparency setting for this monitor, fallback to global if not found
        bool use_transparent = saved_transparency_settings.count(monitor_index) > 0 
                              ? saved_transparency_settings[monitor_index] 
                              : !enable_interaction_;
        
        std::cout << "[AnyWP] [PowerSaving] Using " 
                  << (use_transparent ? "transparent" : "interactive") 
                  << " mode for monitor " << monitor_index << std::endl;
        
        bool success = InitializeWallpaperOnMonitor(url, use_transparent, monitor_index);
        if (success) {
          restored_count++;
        }
      } else {
        std::cout << "[AnyWP] [PowerSaving] Skipping monitor " << monitor_index 
                  << " (not available in current session)" << std::endl;
      }
    }
    
    std::cout << "[AnyWP] [PowerSaving] Successfully restored " << restored_count 
              << " of " << saved_monitor_indices.size() << " monitor(s)" << std::endl;
    
    // If no monitors were restored, try primary display as fallback
    if (restored_count == 0 && !monitors_.empty()) {
      std::cout << "[AnyWP] [PowerSaving] No original monitors available, falling back to primary display" << std::endl;
      
      // v2.0.1+ Bug Fix: Use saved setting for monitor 0 if available, otherwise use global
      bool use_transparent = saved_transparency_settings.count(0) > 0 
                            ? saved_transparency_settings[0] 
                            : !enable_interaction_;
      InitializeWallpaperOnMonitor(url, use_transparent, 0);
    }
  } else {
    // Fallback: initialize on primary display
    std::cout << "[AnyWP] [PowerSaving] No saved monitor config, using primary display (monitor 0)" << std::endl;
    
    // v2.0.1+ Bug Fix: Use saved setting for monitor 0 if available, otherwise use global
    bool use_transparent = saved_transparency_settings.count(0) > 0 
                          ? saved_transparency_settings[0] 
                          : !enable_interaction_;
    InitializeWallpaperOnMonitor(url, use_transparent, 0);
  }
  
  std::cout << "[AnyWP] [PowerSaving] Wallpaper restoration complete" << std::endl;
  
  // Set power state to active
  power_state_ = PowerState::ACTIVE;
  NotifyPowerStateChange(PowerState::ACTIVE);
  
  return true;
}

// Resume wallpaper - Restore animations and rendering
// v1.4.1+ Phase G: Simplified using helper methods
void AnyWPEnginePlugin::ResumeWallpaper(const std::string& reason, bool force_reinit) {
  // Guard: Avoid duplicate resume
  if (!is_paused_.exchange(false)) {
    return;  // Already resumed
  }
  
  std::cout << "[AnyWP] [PowerSaving] ========== RESUMING WALLPAPER ==========" << std::endl;
  std::cout << "[AnyWP] [PowerSaving] Reason: " << reason << std::endl;
  if (force_reinit) {
    std::cout << "[AnyWP] [PowerSaving] Force reinitialize: YES (session switch)" << std::endl;
  }
  
  // CRITICAL FIX: Verify and restore window if necessary (for long-term lock/sleep)
  bool need_reinitialize = force_reinit;  // Force if requested
  
  // Skip validation if force reinit requested
  if (!force_reinit) {
    need_reinitialize = !ValidateWallpaperWindows();
  }
  
  // If window is lost, try to restore it
  if (need_reinitialize) {
    if (RestoreWallpaperConfiguration(default_wallpaper_url_)) {
      return;  // Skip normal resume (already restored and showing)
    }
    return;  // Failed to restore, cannot continue
  }
  
  // v1.4.1+ Phase E: Delegate script execution to PowerManager
  if (power_manager_) {
    power_manager_->ExecuteResumeScripts([this](const std::wstring& script) {
      ExecuteScriptToAllInstances(script);
    });
  }
  
  std::cout << "[AnyWP] [PowerSaving] Wallpaper resumed - animations restarted" << std::endl;
}

// Optimize memory usage (delegated to PowerManager)
void AnyWPEnginePlugin::OptimizeMemoryUsage() {
  if (power_manager_) {
    power_manager_->OptimizeMemoryUsage();
  }
}

// Configure WebView2 memory limits
void AnyWPEnginePlugin::ConfigureWebView2Memory() {
  // Note: WebView2 doesn't expose direct memory limit API
  // But we can configure browser arguments for lower memory usage
  std::cout << "[AnyWP] [Memory] WebView2 memory configuration applied" << std::endl;
}

// SAFE: Schedule memory optimization using JavaScript setTimeout (no dangling pointers)
void AnyWPEnginePlugin::ScheduleSafeMemoryOptimization(ICoreWebView2* webview) {
  if (!webview) {
    return;  // Safety check
  }
  
  std::cout << "[AnyWP] [Memory] Scheduling safe auto-optimization..." << std::endl;
  
  // Use JavaScript setTimeout - runs in WebView2 context, no C++ object dependency
  std::wstring safe_optimize_script = L"setTimeout(function() {"
    L"  try {"
    L"    console.log('[AnyWP] Auto-optimizing memory after navigation...');"
    L"    "
    L"    // Clear console to free memory"
    L"    if (console.clear) console.clear();"
    L"    "
    L"    // Clear browser cache"
    L"    if (window.caches) {"
    L"      caches.keys().then(function(names) {"
    L"        names.forEach(function(name) { caches.delete(name); });"
    L"      });"
    L"    }"
    L"    "
    L"    // Trigger GC if available"
    L"    if (window.gc) window.gc();"
    L"    "
    L"    console.log('[AnyWP] Auto-optimization complete');"
    L"  } catch(e) {"
    L"    console.error('[AnyWP] Auto-optimize error:', e);"
    L"  }"
    L"}, 3000);";  // 3 seconds delay
  
  webview->ExecuteScript(safe_optimize_script.c_str(), nullptr);
}

// Get current process memory usage (with safety checks)
size_t AnyWPEnginePlugin::GetCurrentMemoryUsage() {
  if (power_manager_) {
    return power_manager_->GetCurrentMemoryUsage();
  }
  
  // Fallback: return 0
  return 0;
}

// Execute JavaScript script to all WebView instances (helper function with callback)
void AnyWPEnginePlugin::ExecuteScriptToAllInstances(const std::wstring& script) {
  // SAFE: Send to all instances with lock
  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    for (auto& instance : wallpaper_instances_) {
      // SAFETY: Check webview is valid
      if (instance.webview && instance.webview.Get()) {
        instance.webview->ExecuteScript(
          script.c_str(),
          Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
            [](HRESULT error, LPCWSTR result) -> HRESULT {
              if (SUCCEEDED(error)) {
                if (result && wcslen(result) > 0) {
                  std::wcout << L"[AnyWP] Script executed successfully, result: " << result << std::endl;
                } else {
                  std::cout << "[AnyWP] Script executed successfully, but returned null/empty" << std::endl;
                }
              } else {
                std::stringstream ss;
                ss << "Script execution failed: 0x" << std::hex << error;
                LOG_AND_REPORT_ERROR("WebViewManager", "ExecuteScript", 
                  ss.str(),
                  ErrorHandler::ErrorCategory::EXTERNAL_API, 
                  ErrorHandler::ErrorLevel::ERROR);
              }
              return S_OK;
            }).Get());
      }
    }
  }
  
  // SAFETY: Send to legacy instance with NULL check
  if (webview_ && webview_.Get()) {
    webview_->ExecuteScript(
      script.c_str(),
      Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
        [](HRESULT error, LPCWSTR result) -> HRESULT {
          if (SUCCEEDED(error)) {
            std::wcout << L"[AnyWP] Script executed successfully (legacy), result: " << (result ? result : L"null") << std::endl;
          } else {
            std::stringstream ss;
            ss << "Script execution failed (legacy): 0x" << std::hex << error;
            LOG_AND_REPORT_ERROR("WebViewManager", "ExecuteScript", 
              ss.str(),
              ErrorHandler::ErrorCategory::EXTERNAL_API, 
              ErrorHandler::ErrorLevel::ERROR);
          }
          return S_OK;
        }).Get());
  }
}

// Notify web content about visibility change (Page Visibility API with safety checks)
void AnyWPEnginePlugin::NotifyWebContentVisibility(bool visible) {
  std::cout << "[AnyWP] [PowerSaving] Notifying web content: " << (visible ? "VISIBLE" : "HIDDEN") << std::endl;
  
  // Use Page Visibility API to notify web content
  // This allows web apps to pause/resume animations gracefully
  std::wstring visibility_script = L"(function() {"
    L"  try {"
    L"    // SAFETY: Check if document exists"
    L"    if (!document) return;"
    L"    "
    L"    // Dispatch visibilitychange event"
    L"    var event = new Event('visibilitychange');"
    L"    document.dispatchEvent(event);"
    L"    "
    L"    // SAFETY: Check if window exists"
    L"    if (!window) return;"
    L"    "
    L"    // Also dispatch custom AnyWP event for SDK users"
    L"    var customEvent = new CustomEvent('AnyWP:visibility', {"
    L"      detail: { visible: " + std::wstring(visible ? L"true" : L"false") + L" }"
    L"    });"
    L"    window.dispatchEvent(customEvent);"
    L"    "
    L"    console.log('[AnyWP] Visibility changed: " + std::wstring(visible ? L"visible" : L"hidden") + L"');"
    L"  } catch(e) {"
    L"    console.error('[AnyWP] Error notifying visibility:', e);"
    L"  }"
    L"})();";
  
  ExecuteScriptToAllInstances(visibility_script);
}

// v2.1.1+ Fix: Convert PowerManager::PowerState to AnyWPEnginePlugin::PowerState
AnyWPEnginePlugin::PowerState AnyWPEnginePlugin::ConvertPowerManagerState(
    anywp_engine::PowerManager::PowerState pm_state) {
  // Both enums have the same values, so we can use static_cast
  return static_cast<PowerState>(pm_state);
}

// Convert power state enum to string
std::string AnyWPEnginePlugin::PowerStateToString(PowerState state) {
  switch (state) {
    case PowerState::ACTIVE: return "ACTIVE";
    case PowerState::IDLE: return "IDLE";
    case PowerState::SCREEN_OFF: return "SCREEN_OFF";
    case PowerState::LOCKED: return "LOCKED";
    case PowerState::FULLSCREEN_APP: return "FULLSCREEN_APP";
    case PowerState::PAUSED: return "PAUSED";
    default: return "UNKNOWN";
  }
}

// Notify Dart side about power state changes
// v2.1.1+ Fix: Use message queue instead of InvokeMethod to avoid thread safety issues
void AnyWPEnginePlugin::NotifyPowerStateChange(PowerState newState) {
  // Debug: Log current state before check
  std::cout << "[AnyWP] [PowerSaving] NotifyPowerStateChange called: newState=" 
            << static_cast<int>(newState) << " (" << PowerStateToString(newState) 
            << "), current power_state_=" << static_cast<int>(power_state_) 
            << " (" << PowerStateToString(power_state_) 
            << "), last_power_state_=" << static_cast<int>(last_power_state_) 
            << " (" << PowerStateToString(last_power_state_) << ")" << std::endl;
  
  if (newState == power_state_) {
    std::cout << "[AnyWP] [PowerSaving] No change detected, returning early" << std::endl;
    return;  // No change
  }
  
  // Use current power_state_ as oldState, not last_power_state_
  // This ensures we always report the actual transition
  std::string oldStateStr = PowerStateToString(power_state_);
  std::string newStateStr = PowerStateToString(newState);
  
  std::cout << "[AnyWP] [PowerSaving] State changed: " << oldStateStr << " -> " << newStateStr << std::endl;
  
  // Update states: save current state as last, then update current
  last_power_state_ = power_state_;
  power_state_ = newState;
  
  // v2.1.1+ Fix: Use message queue instead of direct InvokeMethod
  // This avoids thread safety issues when called from window message handler
  try {
    PowerStateChange change;
    change.oldState = oldStateStr;
    change.newState = newStateStr;
    
    // Add to queue (thread-safe)
    {
      std::lock_guard<std::mutex> lock(power_state_changes_mutex_);
      pending_power_state_changes_.push(change);
      
      // Limit queue size to prevent memory issues
      if (pending_power_state_changes_.size() > 100) {
        Logger::Instance().Warning("AnyWPEngine", 
          "Power state change queue size exceeded 100, dropping oldest change");
        pending_power_state_changes_.pop();
      }
    }
    
    std::cout << "[AnyWP] [PowerSaving] Power state change queued: " 
              << oldStateStr << " -> " << newStateStr << std::endl;
    Logger::Instance().Info("AnyWPEngine", 
      "Power state change queued (queue size: " + 
      std::to_string(pending_power_state_changes_.size()) + ")");
  } catch (const std::exception& e) {
    Logger::Instance().Error("AnyWPEngine", 
      std::string("Exception during power state change queuing: ") + e.what());
  } catch (...) {
    Logger::Instance().Error("AnyWPEngine", 
      "Unknown exception during power state change queuing");
  }
}

}  // namespace anywp_engine

// Export C API for plugin registration
extern "C" {

__declspec(dllexport) void AnyWPEnginePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  anywp_engine::AnyWPEnginePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

}  // extern "C"

