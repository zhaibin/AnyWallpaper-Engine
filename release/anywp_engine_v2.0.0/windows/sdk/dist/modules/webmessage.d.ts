/**
 * WebMessage Handler Module
 *
 * Handles all communication with the C++ native layer via chrome.webview.postMessage
 * This is a critical module that must be initialized EARLY to catch all messages.
 *
 * Key responsibilities:
 * - Set up WebMessage listener for C++ → JavaScript communication
 * - Convert screen coordinates to viewport coordinates (DPI scaling)
 * - Dispatch real MouseEvents to DOM for proper event handling
 * - Find interactive elements at click coordinates
 * - Handle CustomEvent dispatching for backward compatibility
 */
/**
 * Initialize WebMessage listener (must be called IMMEDIATELY after script load)
 */
export declare function setupWebMessageListener(): void;
/**
 * WebMessage Module
 */
export declare const WebMessage: {
    setup: typeof setupWebMessageListener;
};
//# sourceMappingURL=webmessage.d.ts.map