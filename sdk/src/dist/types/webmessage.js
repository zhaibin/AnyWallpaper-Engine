/**
 * WebMessage Types
 *
 * Type definitions for C++ <-> JavaScript communication via chrome.webview
 */
/**
 * Type guard to check if data is MouseEventData
 */
export function isMouseEventData(data) {
    return data.type === 'mouseEvent';
}
/**
 * Type guard to check if data is KeyboardEventData
 */
export function isKeyboardEventData(data) {
    return data.type === 'keyboardEvent';
}
/**
 * Type guard to check if data is VisibilityEventData
 */
export function isVisibilityEventData(data) {
    return data.type === 'visibilityChange';
}
/**
 * Type guard to check if data is LogEventData
 */
export function isLogEventData(data) {
    return data.type === 'log';
}
//# sourceMappingURL=webmessage.js.map