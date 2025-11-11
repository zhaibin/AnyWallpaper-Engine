export const AnyWP = {
    // Properties
    version: '2.0.0',
    dpiScale: window.devicePixelRatio || 1,
    screenWidth: screen.width * (window.devicePixelRatio || 1),
    screenHeight: screen.height * (window.devicePixelRatio || 1),
    // Internal state
    _debugMode: false,
    _clickHandlers: [],
    _mouseCallbacks: [],
    _keyboardCallbacks: [],
    _visibilityCallback: null,
    _mutationObserver: null,
    _resizeObserver: null,
    _spaMode: false,
    _autoRefreshEnabled: true,
    _persistedState: {},
    // Initialize (will be implemented in init.ts)
    _init() {
        throw new Error('_init must be implemented');
    },
    // Logging (will be implemented in index.ts)
    _log(_message, _always) {
        throw new Error('Not implemented');
    },
    log(_message) {
        throw new Error('Not implemented');
    },
    // Placeholder methods (will be implemented by modules)
    enableDebug() {
        throw new Error('Not implemented');
    },
    onClick() {
        throw new Error('Not implemented');
    },
    refreshBounds() {
        throw new Error('Not implemented');
    },
    clearHandlers() {
        throw new Error('Not implemented');
    },
    saveState() {
        throw new Error('Not implemented');
    },
    loadState() {
        throw new Error('Not implemented');
    },
    clearState() {
        throw new Error('Not implemented');
    },
    onMouse() {
        throw new Error('Not implemented');
    },
    onKeyboard() {
        throw new Error('Not implemented');
    },
    onVisibilityChange() {
        throw new Error('Not implemented');
    },
    _notifyVisibilityChange() {
        throw new Error('Not implemented');
    },
    setSPAMode() {
        throw new Error('Not implemented');
    },
    openURL(url) {
        console.log('[AnyWP] Opening URL: ' + url);
        if (window.chrome?.webview) {
            window.chrome.webview.postMessage({
                type: 'openURL',
                url: url
            });
        }
        else {
            console.warn('[AnyWP] Native bridge not available');
            window.open(url, '_blank');
        }
    },
    ready(name) {
        console.log('[AnyWP] Wallpaper ready: ' + name);
        if (window.chrome?.webview) {
            window.chrome.webview.postMessage({
                type: 'ready',
                name: name
            });
        }
    }
};
//# sourceMappingURL=AnyWP.js.map