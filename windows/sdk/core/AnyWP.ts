// AnyWP Core Object Definition
import type { 
  AnyWPSDK, 
  ClickHandlerData, 
  PersistedState,
  MouseCallback,
  KeyboardCallback
} from '../types';

export const AnyWP: AnyWPSDK = {
  // Properties
  version: '2.1.0',
  dpiScale: window.devicePixelRatio || 1,
  screenWidth: screen.width * (window.devicePixelRatio || 1),
  screenHeight: screen.height * (window.devicePixelRatio || 1),
  
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
  _persistedState: {} as PersistedState,
  _onFlutterMessage: null as ((message: any) => void) | null,
  
  // Initialize (will be implemented in init.ts)
  _init(): void {
    throw new Error('_init must be implemented');
  },
  
  // Logging (will be implemented in index.ts)
  _log(_message: string, _always?: boolean): void {
    throw new Error('Not implemented');
  },
  
  log(_message: string): void {
    throw new Error('Not implemented');
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
  },
  
  // ========================================
  // Bidirectional Communication APIs
  // ========================================
  
  /**
   * Send message to Flutter application
   * (Will be implemented in webmessage module)
   */
  sendToFlutter(_type: string, _data?: any): boolean {
    throw new Error('Not implemented - will be injected by webmessage module');
  },
  
  /**
   * Register callback to receive messages from Flutter
   * 
   * @param callback - Function to handle messages from Flutter
   * 
   * @example
   * ```typescript
   * window.AnyWP.onMessage((message) => {
   *   console.log('Received from Flutter:', message.type);
   *   
   *   if (message.type === 'updateCarousel') {
   *     // Update carousel with message.data
   *   }
   * });
   * ```
   */
  onMessage(callback: (message: any) => void): void {
    console.log('[AnyWP] Registering Flutter message handler');
    this._onFlutterMessage = callback;
  }
};

