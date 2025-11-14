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
 * Send message to Flutter
 *
 * Sends a structured message to the Flutter application via chrome.webview.postMessage
 *
 * @param type - Message type (e.g., 'carouselStateChanged', 'wallpaperReady', 'error')
 * @param data - Message data payload
 * @returns true if message was sent, false if bridge not available
 *
 * @example
 * ```typescript
 * // Send carousel state change
 * sendToFlutter('carouselStateChanged', {
 *   currentIndex: 2,
 *   totalImages: 10,
 *   isPlaying: true
 * });
 *
 * // Send error
 * sendToFlutter('error', {
 *   code: 'IMAGE_LOAD_FAILED',
 *   message: 'Failed to load image',
 *   details: { url: 'https://example.com/image.jpg' }
 * });
 * ```
 */
export declare function sendToFlutter(type: string, data?: any): boolean;
/**
 * Setup listener for messages from Flutter
 *
 * Listens for CustomEvent 'AnyWP:message' dispatched by Flutter
 * and allows user code to handle these messages
 */
export declare function setupFlutterMessageListener(): void;
/**
 * WebMessage Module
 */
export declare const WebMessage: {
    setup: typeof setupWebMessageListener;
    setupFlutterListener: typeof setupFlutterMessageListener;
    sendToFlutter: typeof sendToFlutter;
};
//# sourceMappingURL=webmessage.d.ts.map