// AnyWP Engine Plugin - Pure C API Header
// For precompiled DLL integration
// This header exposes only the C registration function,
// hiding all C++ implementation details and dependencies.

#ifndef ANYWP_ENGINE_PLUGIN_C_API_H_
#define ANYWP_ENGINE_PLUGIN_C_API_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Define export macro
#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FLUTTER_PLUGIN_EXPORT __declspec(dllimport)
#endif

// Forward declaration of FlutterDesktopPluginRegistrarRef (opaque pointer)
// This avoids the need to include flutter_plugin_registrar.h
typedef struct FlutterDesktopPluginRegistrar* FlutterDesktopPluginRegistrarRef;

// Plugin registration function
// This is the ONLY function that needs to be called from Flutter app.
// Call this in your app's flutter/generated_plugin_registrant.cc
FLUTTER_PLUGIN_EXPORT void AnyWPEnginePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#ifdef __cplusplus
}
#endif

#endif  // ANYWP_ENGINE_PLUGIN_C_API_H_

