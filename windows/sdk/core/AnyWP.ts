// AnyWP Core Object Definition
import type { 
  AnyWPSDK, 
  ClickHandlerData, 
  DraggableData, 
  DragState, 
  PersistedState,
  MouseCallback,
  KeyboardCallback
} from '../types';

export const AnyWP: AnyWPSDK = {
  // Properties
  version: '4.2.0',
  dpiScale: window.devicePixelRatio || 1,
  screenWidth: screen.width * (window.devicePixelRatio || 1),
  screenHeight: screen.height * (window.devicePixelRatio || 1),
  interactionEnabled: true,
  
  // Internal state
  _debugMode: false,
  _clickHandlers: [] as ClickHandlerData[],
  _mouseCallbacks: [] as MouseCallback[],
  _keyboardCallbacks: [] as KeyboardCallback[],
  _visibilityCallback: null,
  _mutationObserver: null,
  _resizeObserver: null,
  _spaMode: false,
  _autoRefreshEnabled: true,
  _draggableElements: [] as DraggableData[],
  _dragState: null as DragState | null,
  _persistedState: {} as PersistedState,
  
  // Initialize (will be implemented in init.ts)
  _init(): void {
    throw new Error('_init must be implemented');
  },
  
  // Placeholder methods (will be implemented by modules)
  enableDebug(): void {
    throw new Error('Not implemented');
  },
  
  onClick(): void {
    throw new Error('Not implemented');
  },
  
  refreshBounds(): number {
    throw new Error('Not implemented');
  },
  
  clearHandlers(): void {
    throw new Error('Not implemented');
  },
  
  makeDraggable(): void {
    throw new Error('Not implemented');
  },
  
  removeDraggable(): void {
    throw new Error('Not implemented');
  },
  
  resetPosition(): boolean {
    throw new Error('Not implemented');
  },
  
  saveState(): void {
    throw new Error('Not implemented');
  },
  
  loadState(): void {
    throw new Error('Not implemented');
  },
  
  clearState(): void {
    throw new Error('Not implemented');
  },
  
  onMouse(): void {
    throw new Error('Not implemented');
  },
  
  onKeyboard(): void {
    throw new Error('Not implemented');
  },
  
  onVisibilityChange(): void {
    throw new Error('Not implemented');
  },
  
  _notifyVisibilityChange(): void {
    throw new Error('Not implemented');
  },
  
  setSPAMode(): void {
    throw new Error('Not implemented');
  },
  
  openURL(url: string): void {
    console.log('[AnyWP] Opening URL: ' + url);
    
    if (window.chrome?.webview) {
      window.chrome.webview.postMessage({
        type: 'openURL',
        url: url
      });
    } else {
      console.warn('[AnyWP] Native bridge not available');
      window.open(url, '_blank');
    }
  },
  
  ready(name: string): void {
    console.log('[AnyWP] Wallpaper ready: ' + name);
    
    if (window.chrome?.webview) {
      window.chrome.webview.postMessage({
        type: 'ready',
        name: name
      });
    }
  }
};

