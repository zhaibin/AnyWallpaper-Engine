// AnyWP SDK Type Definitions

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
 * Drag event data
 */
export interface DragEventData {
  x: number;
  y: number;
  deltaX?: number;
  deltaY?: number;
}

/**
 * Drag state
 */
export interface DragState {
  element: HTMLElement;
  data: DraggableData;
  startX: number;
  startY: number;
  offsetX: number;
  offsetY: number;
  initialLeft: number;
  initialTop: number;
  windowLeft: number;
  windowTop: number;
  docLeft: number;
  docTop: number;
  dpi: number;
}

/**
 * Bounds constraints for dragging
 */
export interface BoundsConstraint {
  left?: number;
  top?: number;
  right?: number;
  bottom?: number;
}

/**
 * Draggable element options
 */
export interface DraggableOptions {
  persistKey?: string;
  onDragStart?: (pos: Position) => void;
  onDrag?: (data: DragEventData) => void;
  onDragEnd?: (pos: Position) => void;
  bounds?: BoundsConstraint;
}

/**
 * Draggable element data
 */
export interface DraggableData {
  element: HTMLElement;
  persistKey: string | null;
  onDragStart: ((pos: Position) => void) | null;
  onDrag: ((data: DragEventData) => void) | null;
  onDragEnd: ((pos: Position) => void) | null;
  bounds: BoundsConstraint | null;
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
  positionCheckTimer?: any;  // NodeJS.Timeout | number (browser vs node)
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
  [key: string]: any;
}

/**
 * Persisted state data
 */
export type PersistedState = Record<string, any>;

/**
 * Callback types
 */
export type ClickCallback = (x: number, y: number) => void;
export type MouseCallback = (detail: MouseEventDetail) => void;
export type KeyboardCallback = (detail: KeyboardEventDetail) => void;
export type VisibilityCallback = (visible: boolean) => void;
export type StateLoadCallback = (data: any) => void;

/**
 * Main AnyWP SDK interface
 */
export interface AnyWPSDK {
  // Properties
  version: string;
  dpiScale: number;
  screenWidth: number;
  screenHeight: number;
  interactionEnabled: boolean;
  
  // Internal state
  _debugMode: boolean;
  _clickHandlers: ClickHandlerData[];
  _mouseCallbacks: MouseCallback[];
  _keyboardCallbacks: KeyboardCallback[];
  _visibilityCallback: VisibilityCallback | null;
  _mutationObserver: MutationObserver | null;
  _resizeObserver: ResizeObserver | null;
  _spaMode: boolean;
  _autoRefreshEnabled: boolean;
  _draggableElements: DraggableData[];
  _dragState: DragState | null;
  _persistedState: PersistedState;
  
  // Methods
  _init(): void;
  _log(message: string, always?: boolean): void;
  log(message: string): void;
  enableDebug(): void;
  onClick(element: string | HTMLElement, callback: ClickCallback, options?: ClickHandlerOptions): void;
  refreshBounds(): number;
  clearHandlers(): void;
  makeDraggable(element: string | HTMLElement, options?: DraggableOptions): void;
  removeDraggable(element: string | HTMLElement): void;
  resetPosition(element: string | HTMLElement, position?: Position): boolean;
  saveState(key: string, value: any): void;
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

