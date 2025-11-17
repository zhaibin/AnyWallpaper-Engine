// =============================================================================
// AnyWP Engine - SDK Loader from Embedded Resource
// =============================================================================

#include "sdk_loader.h"
#include "sdk_resource.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

namespace anywp {
namespace sdk {

// Load SDK from embedded DLL resource
std::string LoadSDKFromResource(HMODULE hModule) {
    try {
        // If module handle not provided, get current DLL handle
        if (!hModule) {
            hModule = GetModuleHandleW(L"anywp_engine_plugin.dll");
            if (!hModule) {
                std::cerr << "[AnyWP] [SDK] Failed to get DLL module handle" << std::endl;
                return "";
            }
        }
        
        // Find resource
        HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_ANYWP_SDK), RT_RCDATA);
        if (!hResource) {
            DWORD error = GetLastError();
            std::cerr << "[AnyWP] [SDK] Failed to find embedded SDK resource (Error: " 
                      << error << ")" << std::endl;
            return "";
        }
        
        // Load resource
        HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
        if (!hLoadedResource) {
            DWORD error = GetLastError();
            std::cerr << "[AnyWP] [SDK] Failed to load SDK resource (Error: " 
                      << error << ")" << std::endl;
            return "";
        }
        
        // Lock resource to get pointer
        LPVOID pLockedResource = LockResource(hLoadedResource);
        if (!pLockedResource) {
            std::cerr << "[AnyWP] [SDK] Failed to lock SDK resource" << std::endl;
            return "";
        }
        
        // Get resource size
        DWORD dwResourceSize = SizeofResource(hModule, hResource);
        if (dwResourceSize == 0) {
            std::cerr << "[AnyWP] [SDK] SDK resource size is 0" << std::endl;
            return "";
        }
        
        // Convert to std::string
        std::string sdkContent(static_cast<const char*>(pLockedResource), dwResourceSize);
        
        std::cout << "[AnyWP] [SDK] Loaded embedded SDK successfully (" 
                  << dwResourceSize << " bytes)" << std::endl;
        
        return sdkContent;
    }
    catch (const std::exception& e) {
        std::cerr << "[AnyWP] [SDK] Exception in LoadSDKFromResource: " << e.what() << std::endl;
        return "";
    }
    catch (...) {
        std::cerr << "[AnyWP] [SDK] Unknown exception in LoadSDKFromResource" << std::endl;
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
            std::cout << "[AnyWP] [SDK] Loaded SDK from file: " << path 
                      << " (" << size << " bytes)" << std::endl;
            return content;
        }
        
        return "";
    }
    catch (const std::exception& e) {
        std::cerr << "[AnyWP] [SDK] Exception in LoadSDKFromFile: " << e.what() << std::endl;
        return "";
    }
    catch (...) {
        std::cerr << "[AnyWP] [SDK] Unknown exception in LoadSDKFromFile" << std::endl;
        return "";
    }
}

// Smart SDK loader (prioritizes resource, falls back to file)
std::string LoadSDKScript(HMODULE hModule) {
    try {
        // Strategy 1: Load from embedded resource (recommended)
        std::string sdkContent = LoadSDKFromResource(hModule);
        if (!sdkContent.empty()) {
            std::cout << "[AnyWP] [SDK] Using embedded SDK (from DLL resource)" << std::endl;
            return sdkContent;
        }
        
        std::cout << "[AnyWP] [SDK] Embedded SDK not available, trying file fallback..." << std::endl;
        
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
        
        std::cerr << "[AnyWP] [SDK] ERROR: Failed to load SDK from both resource and file system!" << std::endl;
        std::cerr << "[AnyWP] [SDK] Please ensure SDK is embedded in DLL or available at runtime." << std::endl;
        
        return "";
    }
    catch (const std::exception& e) {
        std::cerr << "[AnyWP] [SDK] Exception in LoadSDKScript: " << e.what() << std::endl;
        return "";
    }
    catch (...) {
        std::cerr << "[AnyWP] [SDK] Unknown exception in LoadSDKScript" << std::endl;
        return "";
    }
}

} // namespace sdk
} // namespace anywp

