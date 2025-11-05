// Precompiled plugin stub header
// This file is required by Flutter's plugin registration system

#ifndef FLUTTER_PLUGIN_ANY_W_P_ENGINE_PLUGIN_H_
#define FLUTTER_PLUGIN_ANY_W_P_ENGINE_PLUGIN_H_

#include <flutter/plugin_registrar_windows.h>

#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FLUTTER_PLUGIN_EXPORT __declspec(dllimport)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

FLUTTER_PLUGIN_EXPORT void AnyWPEnginePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_PLUGIN_ANY_W_P_ENGINE_PLUGIN_H_


