// AnyWP Engine SDK - Main Entry Point
// Modular architecture with TypeScript support
import { AnyWP } from './core/AnyWP';
import { initializeAnyWP } from './core/init';
import { Debug } from './utils/debug';
import { Events } from './modules/events';
import { ClickHandler } from './modules/click';
import { Storage } from './modules/storage';
import { SPA } from './modules/spa';
import { WebMessage } from './modules/webmessage';
// Implement initialization
AnyWP._init = function () {
    initializeAnyWP(this);
};
// Public API: Debug
AnyWP.enableDebug = function () {
    Debug.enable();
};
AnyWP._log = function (message, always) {
    Debug.log(message, always);
};
// Public API: Debug
AnyWP.log = function (message) {
    Debug.log(message, true);
    // Also send to native if available
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
            type: 'log',
            message: message
        });
    }
};
// Public API: Click Handler
AnyWP.onClick = function (element, callback, options) {
    ClickHandler.onClick(this, element, callback, options);
};
AnyWP.refreshBounds = function () {
    return ClickHandler.refreshBounds(this);
};
AnyWP.clearHandlers = function () {
    ClickHandler.clearHandlers(this);
};
// Public API: Storage
AnyWP.saveState = function (key, value) {
    Storage.save(this, key, value);
};
AnyWP.loadState = function (key, callback) {
    Storage.load(this, key, callback);
};
AnyWP.clearState = function () {
    Storage.clear(this);
};
// Public API: Events
AnyWP.onMouse = function (callback) {
    Events.onMouse(this, callback);
};
AnyWP.onKeyboard = function (callback) {
    Events.onKeyboard(this, callback);
};
AnyWP.onVisibilityChange = function (callback) {
    Events.onVisibilityChange(this, callback);
};
AnyWP._notifyVisibilityChange = function (visible) {
    Events.notifyVisibilityChange(this, visible);
};
// Public API: SPA
AnyWP.setSPAMode = function (enabled) {
    SPA.setSPAMode(this, ClickHandler, enabled);
};
// Note: openURL and ready are implemented in core/AnyWP.ts
// Export for build
export { AnyWP };
// Auto-initialize when DOM is ready
if (typeof window !== 'undefined') {
    // ========== CRITICAL: Setup WebMessage listener IMMEDIATELY ==========
    // This must be done BEFORE any other initialization to catch all messages from C++
    WebMessage.setup();
    // ========== CRITICAL: Prevent Duplicate SDK Initialization ==========
    // Check if SDK is already loaded (防止重复注入)
    if (typeof window.AnyWP !== 'undefined') {
        console.info('[AnyWP] SDK already loaded, skipping re-initialization');
        console.info('[AnyWP] This is expected when C++ plugin injects SDK multiple times');
    }
    else {
        console.info('[AnyWP] Initializing SDK for the first time');
        window.AnyWP = AnyWP;
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', function () {
                AnyWP._init();
            });
        }
        else {
            AnyWP._init();
        }
        console.info('[AnyWP] SDK loaded successfully');
    }
}
//# sourceMappingURL=index.js.map