/**
 * WebMessage Types
 *
 * Type definitions for C++ <-> JavaScript communication via chrome.webview
 */
/**
 * Mouse event data from C++
 */
export interface MouseEventData {
    type: 'mouseEvent';
    eventType: 'mousedown' | 'mouseup' | 'click' | 'mousemove';
    x: number;
    y: number;
    button?: number;
}
/**
 * Keyboard event data from C++
 */
export interface KeyboardEventData {
    type: 'keyboardEvent';
    eventType: 'keydown' | 'keyup';
    key: string;
    code: string;
    ctrlKey?: boolean;
    shiftKey?: boolean;
    altKey?: boolean;
}
/**
 * Visibility change event data from C++
 */
export interface VisibilityEventData {
    type: 'visibilityChange';
    visible: boolean;
}
/**
 * General log message from C++
 */
export interface LogEventData {
    type: 'log';
    message: string;
    level?: 'error' | 'warn' | 'info' | 'debug';
}
/**
 * PowerStateChange message data (v2.1.7+)
 */
export interface PowerStateChangeData {
    type: 'powerStateChange';
    visible: boolean;
    reason?: string;
}
/**
 * Union type of all possible WebMessage data
 */
export type WebMessageData = MouseEventData | KeyboardEventData | VisibilityEventData | LogEventData | PowerStateChangeData;
/**
 * WebMessage event from chrome.webview
 */
export interface WebMessageEvent extends MessageEvent {
    data: WebMessageData;
}
/**
 * Type guard to check if data is MouseEventData
 */
export declare function isMouseEventData(data: WebMessageData): data is MouseEventData;
/**
 * Type guard to check if data is KeyboardEventData
 */
export declare function isKeyboardEventData(data: WebMessageData): data is KeyboardEventData;
/**
 * Type guard to check if data is VisibilityEventData
 */
export declare function isVisibilityEventData(data: WebMessageData): data is VisibilityEventData;
/**
 * Type guard to check if data is LogEventData
 */
export declare function isLogEventData(data: WebMessageData): data is LogEventData;
//# sourceMappingURL=webmessage.d.ts.map