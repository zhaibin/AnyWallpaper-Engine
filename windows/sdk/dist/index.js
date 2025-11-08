// AnyWP Engine SDK - Main Entry Point
// Modular architecture with TypeScript support
import { AnyWP } from './core/AnyWP';
import { initializeAnyWP } from './core/init';
import { Debug } from './utils/debug';
import { Events } from './modules/events';
import { ClickHandler } from './modules/click';
import { DragHandler } from './modules/drag';
import { Storage } from './modules/storage';
import { SPA } from './modules/spa';
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
// Public API: Drag Handler
AnyWP.makeDraggable = function (element, options) {
    DragHandler.makeDraggable(this, element, options);
};
AnyWP.removeDraggable = function (element) {
    DragHandler.removeDraggable(this, element);
};
AnyWP.resetPosition = function (element, position) {
    return DragHandler.resetPosition(this, element, position ? { left: position.x, top: position.y } : undefined);
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
// Public API: Utility
AnyWP.openURL = function (url) {
    Debug.log('Opening URL: ' + url);
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
            type: 'openURL',
            url: url
        });
    }
    else {
        console.warn('[AnyWP] Native bridge not available');
        window.open(url, '_blank');
    }
};
AnyWP.ready = function (name) {
    Debug.log('Wallpaper ready: ' + name, true);
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
            type: 'ready',
            name: name
        });
    }
};
// Export for build
export { AnyWP };
// Auto-initialize when DOM is ready
if (typeof window !== 'undefined') {
    window.AnyWP = AnyWP;
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function () {
            AnyWP._init();
        });
    }
    else {
        AnyWP._init();
    }
    console.log('[AnyWP] SDK loaded successfully');
}
//# sourceMappingURL=index.js.map