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
  x: number;         // Physical screen X coordinate
  y: number;         // Physical screen Y coordinate
  button?: number;   // Mouse button (0=left, 1=middle, 2=right)
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
export type WebMessageData = 
  | MouseEventData 
  | KeyboardEventData 
  | VisibilityEventData 
  | LogEventData
  | PowerStateChangeData;

/**
 * WebMessage event from chrome.webview
 */
export interface WebMessageEvent extends MessageEvent {
  data: WebMessageData;
}

/**
 * Type guard to check if data is MouseEventData
 */
export function isMouseEventData(data: WebMessageData): data is MouseEventData {
  return data.type === 'mouseEvent';
}

/**
 * Type guard to check if data is KeyboardEventData
 */
export function isKeyboardEventData(data: WebMessageData): data is KeyboardEventData {
  return data.type === 'keyboardEvent';
}

/**
 * Type guard to check if data is VisibilityEventData
 */
export function isVisibilityEventData(data: WebMessageData): data is VisibilityEventData {
  return data.type === 'visibilityChange';
}

/**
 * Type guard to check if data is LogEventData
 */
export function isLogEventData(data: WebMessageData): data is LogEventData {
  return data.type === 'log';
}


