// AnyWP Core Object Definition
export const AnyWP = {
  version: '4.2.0',
  dpiScale: window.devicePixelRatio || 1,
  screenWidth: screen.width * (window.devicePixelRatio || 1),
  screenHeight: screen.height * (window.devicePixelRatio || 1),
  interactionEnabled: true,
  
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
  _draggableElements: [],
  _dragState: null,
  _persistedState: {},
  
  // Initialize (will be implemented in init.js)
  _init() {
    throw new Error('_init must be implemented');
  }
};

