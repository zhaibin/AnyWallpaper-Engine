// =============================================================================
// AnyWP Engine - SDK Loader Header
// =============================================================================

#ifndef ANYWP_SDK_LOADER_H
#define ANYWP_SDK_LOADER_H

#include <windows.h>
#include <string>

namespace anywp {
namespace sdk {

/// @brief Load SDK from embedded DLL resource
/// @param hModule DLL module handle (nullptr to auto-detect)
/// @return SDK JavaScript content or empty string on failure
std::string LoadSDKFromResource(HMODULE hModule = nullptr);

/// @brief Load SDK from file system (fallback)
/// @param path Path to SDK file
/// @return SDK JavaScript content or empty string on failure
std::string LoadSDKFromFile(const std::string& path);

/// @brief Smart SDK loader (prioritizes resource, falls back to file)
/// @param hModule DLL module handle (nullptr to auto-detect)
/// @return SDK JavaScript content or empty string on failure
std::string LoadSDKScript(HMODULE hModule = nullptr);

} // namespace sdk
} // namespace anywp

#endif // ANYWP_SDK_LOADER_H

