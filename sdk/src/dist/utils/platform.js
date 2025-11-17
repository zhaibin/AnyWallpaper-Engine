/**
 * Platform Detection and Message Bridge Abstraction
 *
 * Provides unified interface for communication with native layer
 * across different platforms (Windows WebView2, macOS WKWebView)
 */
/**
 * Windows WebView2 Bridge
 */
class WindowsBridge {
    constructor() {
        this.platform = 'windows';
        this.handlers = new Set();
    }
    isAvailable() {
        return !!window.chrome?.webview;
    }
    postMessage(message) {
        if (!this.isAvailable()) {
            throw new Error('Windows WebView2 bridge not available');
        }
        window.chrome.webview.postMessage(message);
    }
    addEventListener(handler) {
        if (!this.isAvailable()) {
            throw new Error('Windows WebView2 bridge not available');
        }
        const wrappedHandler = (event) => {
            handler(event.data);
        };
        this.handlers.add(handler);
        window.chrome.webview.addEventListener('message', wrappedHandler);
    }
    removeEventListener(handler) {
        // Note: In Windows, we need to keep reference to wrapped handler
        // This is a simplified version
        this.handlers.delete(handler);
    }
}
/**
 * macOS WKWebView Bridge
 */
class MacOSBridge {
    constructor() {
        this.platform = 'macos';
        this.handlers = new Set();
    }
    isAvailable() {
        return !!window.webkit?.messageHandlers?.anywpMessage;
    }
    postMessage(message) {
        if (!this.isAvailable()) {
            throw new Error('macOS WKWebView bridge not available');
        }
        // WKWebView expects string message
        const messageStr = typeof message === 'string' ? message : JSON.stringify(message);
        window.webkit.messageHandlers.anywpMessage.postMessage(messageStr);
    }
    addEventListener(handler) {
        if (!this.isAvailable()) {
            throw new Error('macOS WKWebView bridge not available');
        }
        // For macOS, we use window.postMessage for native -> JS communication
        const wrappedHandler = (event) => {
            // Filter out messages not from our native bridge
            if (event.source !== window) {
                return;
            }
            handler(event.data);
        };
        this.handlers.add(handler);
        window.addEventListener('message', wrappedHandler);
    }
    removeEventListener(handler) {
        this.handlers.delete(handler);
        // Note: In production, we'd need to keep reference to wrapped handler
    }
}
/**
 * Detect current platform
 */
export function detectPlatform() {
    // Check Windows WebView2
    if (window.chrome?.webview) {
        return 'windows';
    }
    // Check macOS WKWebView
    if (window.webkit?.messageHandlers?.anywpMessage) {
        return 'macos';
    }
    return 'unknown';
}
/**
 * Get platform-specific bridge
 */
export function getPlatformBridge() {
    const platform = detectPlatform();
    switch (platform) {
        case 'windows':
            return new WindowsBridge();
        case 'macos':
            return new MacOSBridge();
        default:
            throw new Error(`Unsupported platform: ${platform}`);
    }
}
/**
 * Global platform bridge instance (singleton)
 */
let bridgeInstance = null;
/**
 * Get or create platform bridge instance
 */
export function getBridge() {
    if (!bridgeInstance) {
        bridgeInstance = getPlatformBridge();
    }
    return bridgeInstance;
}
/**
 * Check if running in AnyWP Engine
 */
export function isAnyWP() {
    const platform = detectPlatform();
    return platform !== 'unknown';
}
//# sourceMappingURL=platform.js.map