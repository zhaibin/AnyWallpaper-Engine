/**
 * Element bounds in physical pixels
 */
export interface ElementBounds {
    left: number;
    top: number;
    right: number;
    bottom: number;
    width: number;
    height: number;
}
/**
 * Position coordinates
 */
export interface Position {
    x: number;
    y: number;
}
/**
 * Coordinate offsets
 */
export interface CoordinateOffsets {
    windowLeft: number;
    windowTop: number;
    docLeft: number;
    docTop: number;
    dpi: number;
}
/**
 * Click handler options
 */
export interface ClickHandlerOptions {
    immediate?: boolean;
    waitFor?: boolean;
    maxWait?: number;
    autoRefresh?: boolean;
    delay?: number;
    debug?: boolean;
}
/**
 * Click handler data
 */
export interface ClickHandlerData {
    element: HTMLElement;
    callback: (x: number, y: number) => void;
    bounds: ElementBounds;
    selector: string | null;
    autoRefresh: boolean;
    options: ClickHandlerOptions;
    resizeObserver?: ResizeObserver;
    intersectionObserver?: IntersectionObserver;
    positionCheckTimer?: any;
    lastBounds?: ElementBounds;
}
/**
 * Mouse event detail
 */
export interface MouseEventDetail {
    type: 'mousedown' | 'mousemove' | 'mouseup';
    x: number;
    y: number;
}
/**
 * Keyboard event detail
 */
export interface KeyboardEventDetail {
    type: 'keydown' | 'keyup';
    key: string;
    code: string;
}
/**
 * Visibility event detail
 */
export interface VisibilityEventDetail {
    visible: boolean;
}
/**
 * WebView2 message
 */
export interface WebViewMessage {
    type: string;
    key?: string;
    value?: string;
    url?: string;
    name?: string;
    interactive?: boolean;
    message?: string;
}
/**
 * Serializable value types for state persistence
 */
export type StateValue = string | number | boolean | null | StateValue[] | {
    [key: string]: StateValue;
};
/**
 * Persisted state data
 */
export type PersistedState = Record<string, StateValue | null>;
/**
 * Callback types
 */
export type ClickCallback = (x: number, y: number) => void;
export type MouseCallback = (detail: MouseEventDetail) => void;
export type KeyboardCallback = (detail: KeyboardEventDetail) => void;
export type VisibilityCallback = (visible: boolean) => void;
export type StateLoadCallback = (data: StateValue | null) => void;
/**
 * Main AnyWP SDK interface
 */
export interface AnyWPSDK {
    version: string;
    dpiScale: number;
    screenWidth: number;
    screenHeight: number;
    _debugMode: boolean;
    _clickHandlers: ClickHandlerData[];
    _mouseCallbacks: MouseCallback[];
    _keyboardCallbacks: KeyboardCallback[];
    _visibilityCallback: VisibilityCallback | null;
    _mutationObserver: MutationObserver | null;
    _resizeObserver: ResizeObserver | null;
    _spaMode: boolean;
    _autoRefreshEnabled: boolean;
    _persistedState: PersistedState;
    _init(): void;
    _log(message: string, always?: boolean): void;
    log(message: string): void;
    enableDebug(): void;
    onClick(element: string | HTMLElement, callback: ClickCallback, options?: ClickHandlerOptions): void;
    refreshBounds(): number;
    clearHandlers(): void;
    saveState(key: string, value: StateValue): void;
    loadState(key: string, callback: StateLoadCallback): void;
    clearState(): void;
    onMouse(callback: MouseCallback): void;
    onKeyboard(callback: KeyboardCallback): void;
    onVisibilityChange(callback: VisibilityCallback): void;
    _notifyVisibilityChange(visible: boolean): void;
    setSPAMode(enabled: boolean): void;
    openURL(url: string): void;
    ready(name: string): void;
}
/**
 * Global window extension
 */
declare global {
    interface Window {
        AnyWP: AnyWPSDK;
        chrome?: {
            webview?: {
                postMessage(message: WebViewMessage): void;
            };
        };
    }
}
export {};
//# sourceMappingURL=types.d.ts.map