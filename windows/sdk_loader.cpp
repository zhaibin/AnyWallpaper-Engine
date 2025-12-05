// =============================================================================
// AnyWP Engine - SDK Loader from Embedded Resource
// =============================================================================

#include "sdk_loader.h"
#include "sdk_resource.h"
#include "utils/logger.h"
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>

using anywp_engine::Logger;

namespace anywp {
namespace sdk {

// Load SDK from embedded DLL resource
std::string LoadSDKFromResource(HMODULE hModule) {
    try {
        // If module handle not provided, get current DLL handle
        if (!hModule) {
            hModule = GetModuleHandleW(L"anywp_engine_plugin.dll");
            if (!hModule) {
                Logger::Instance().Error("SDK", "Failed to get DLL module handle");
                return "";
            }
        }
        
        // Find resource
        HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_ANYWP_SDK), RT_RCDATA);
        if (!hResource) {
            DWORD error = GetLastError();
            Logger::Instance().Error("SDK", "Failed to find embedded SDK resource (Error: " + std::to_string(error) + ")");
            return "";
        }
        
        // Load resource
        HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
        if (!hLoadedResource) {
            DWORD error = GetLastError();
            Logger::Instance().Error("SDK", "Failed to load SDK resource (Error: " + std::to_string(error) + ")");
            return "";
        }
        
        // Lock resource to get pointer
        LPVOID pLockedResource = LockResource(hLoadedResource);
        if (!pLockedResource) {
            Logger::Instance().Error("SDK", "Failed to lock SDK resource");
            return "";
        }
        
        // Get resource size
        DWORD dwResourceSize = SizeofResource(hModule, hResource);
        if (dwResourceSize == 0) {
            Logger::Instance().Error("SDK", "SDK resource size is 0");
            return "";
        }
        
        // Convert to std::string
        std::string sdkContent(static_cast<const char*>(pLockedResource), dwResourceSize);
        
        Logger::Instance().Info("SDK", 
          "Loaded embedded SDK successfully (" + std::to_string(dwResourceSize) + " bytes)");
        
        return sdkContent;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("SDK", "Exception in LoadSDKFromResource: " + std::string(e.what()));
        return "";
    }
    catch (...) {
        Logger::Instance().Error("SDK", "Unknown exception in LoadSDKFromResource");
        return "";
    }
}

// Load SDK from file system (fallback)
std::string LoadSDKFromFile(const std::string& path) {
    try {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return "";
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::string content(size, '\0');
        if (file.read(&content[0], size)) {
            Logger::Instance().Info("SDK", 
              "Loaded SDK from file: " + path + " (" + std::to_string(size) + " bytes)");
            return content;
        }
        
        return "";
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("SDK", "Exception in LoadSDKFromFile: " + std::string(e.what()));
        return "";
    }
    catch (...) {
        Logger::Instance().Error("SDK", "Unknown exception in LoadSDKFromFile");
        return "";
    }
}

// Smart SDK loader (prioritizes resource, falls back to file)
std::string LoadSDKScript(HMODULE hModule) {
    try {
        // Strategy 1: Load from embedded resource (recommended)
        std::string sdkContent = LoadSDKFromResource(hModule);
        if (!sdkContent.empty()) {
            Logger::Instance().Info("SDK", "Using embedded SDK (from DLL resource)");
            return sdkContent;
        }
        
        Logger::Instance().Info("SDK", "Embedded SDK not available, trying file fallback...");
        
        // Strategy 2: Load from file system (fallback)
        std::vector<std::string> searchPaths = {
            "sdk/dist/anywp_sdk.js",        // v2.2.1+ standard path
            "sdk/anywp_sdk.js",             // Backward compatibility
            "../sdk/dist/anywp_sdk.js",     // Relative path
            "../sdk/anywp_sdk.js",          // Relative path (compatibility)
            "data/flutter_assets/sdk/dist/anywp_sdk.js",  // Flutter assets
        };
        
        for (const auto& path : searchPaths) {
            sdkContent = LoadSDKFromFile(path);
            if (!sdkContent.empty()) {
                return sdkContent;
            }
        }
        
        Logger::Instance().Error("SDK", "Failed to load SDK from both resource and file system!");
        Logger::Instance().Error("SDK", "Please ensure SDK is embedded in DLL or available at runtime.");
        
        return "";
    }
    catch (const std::exception& e) {
        Logger::Instance().Error("SDK", "Exception in LoadSDKScript: " + std::string(e.what()));
        return "";
    }
    catch (...) {
        Logger::Instance().Error("SDK", "Unknown exception in LoadSDKScript");
        return "";
    }
}

} // namespace sdk
} // namespace anywp

