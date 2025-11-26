#include "permission_configurator.h"
#include "../utils/logger.h"

namespace anywp_engine {

PermissionConfigurator::PermissionConfigurator() {
  Logger::Instance().Info("PermissionConfigurator", "Module initialized");
}

PermissionConfigurator::~PermissionConfigurator() {
  Logger::Instance().Info("PermissionConfigurator", "Module destroyed");
}

bool PermissionConfigurator::ConfigurePermissions(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("PermissionConfigurator", "WebView is null");
    return false;
  }

  // Note: Default permissions are sufficient for wallpaper use cases.
  // Custom permission configuration can be added here if needed.
  Logger::Instance().Info("PermissionConfigurator", "Permissions configured (default)");
  return true;
}

bool PermissionConfigurator::SetupSecurityHandlers(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("PermissionConfigurator", "WebView is null");
    return false;
  }

  // Note: WebView2 default security handlers are used.
  // Custom security handlers can be added here for specific requirements.
  Logger::Instance().Info("PermissionConfigurator", "Security handlers setup (default)");
  return true;
}

HRESULT PermissionConfigurator::OnPermissionRequested(
    ICoreWebView2* sender,
    ICoreWebView2PermissionRequestedEventArgs* args) {
  
  // Note: Permission requests are allowed by default for local wallpaper content.
  // Custom handling can be added here for specific permission types.
  Logger::Instance().Debug("PermissionConfigurator", "Permission requested (allowed by default)");
  return S_OK;
}

}  // namespace anywp_engine

