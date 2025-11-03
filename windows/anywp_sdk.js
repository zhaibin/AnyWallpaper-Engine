// AnyWP Engine SDK v4.0.0 - JavaScript Bridge
// Auto-injected into WebView2
// React/Vue SPA Compatible

(function() {
  'use strict';

  // AnyWP Global Object
  window.AnyWP = {
    version: '4.0.0',
    dpiScale: window.devicePixelRatio || 1,
    screenWidth: screen.width * (window.devicePixelRatio || 1),
    screenHeight: screen.height * (window.devicePixelRatio || 1),
    interactionEnabled: false,
    
    // Debug mode
    _debugMode: false,
    _clickHandlers: [],
    _mouseCallbacks: [],
    _keyboardCallbacks: [],
    _mutationObserver: null,
    _resizeObserver: null,
    _spaMode: false,
    _autoRefreshEnabled: true,
    
    // Initialize
    _init: function() {
      console.log('========================================');
      console.log('AnyWP Engine v' + this.version + ' (SPA Compatible)');
      console.log('========================================');
      console.log('Screen: ' + this.screenWidth + 'x' + this.screenHeight);
      console.log('DPI Scale: ' + this.dpiScale + 'x');
      console.log('User Agent: ' + navigator.userAgent);
      console.log('========================================');
      
      // Detect debug mode from URL
      this._detectDebugMode();
      
      // Detect SPA framework
      this._detectSPA();
      
      // Setup event listeners
      this._setupEventListeners();
    },
    
    // Detect SPA framework
    _detectSPA: function() {
      const self = this;
      
      // Immediate check for React/Vue/Angular globals
      const isReact = !!(window.React || window.ReactDOM);
      const isVue = !!(window.Vue);
      const isAngular = !!(window.angular);
      
      if (isReact || isVue || isAngular) {
        self._spaMode = true;
        const framework = isReact ? 'React' : (isVue ? 'Vue' : 'Angular');
        console.log('[AnyWP] SPA Framework detected immediately: ' + framework);
        console.log('[AnyWP] Auto-refresh enabled for dynamic content');
        self._setupSPAMonitoring();
      } else {
        // Delayed check for DOM elements
        setTimeout(function() {
          const isReactDOM = !!document.querySelector('[data-reactroot], [data-reactid], #root');
          const isVueDOM = !!document.querySelector('[data-v-]');
          const isAngularDOM = !!document.querySelector('[ng-version]');
          
          if (isReactDOM || isVueDOM || isAngularDOM) {
            self._spaMode = true;
            const framework = isReactDOM ? 'React' : (isVueDOM ? 'Vue' : 'Angular');
            console.log('[AnyWP] SPA Framework detected via DOM: ' + framework);
            console.log('[AnyWP] Auto-refresh enabled for dynamic content');
            self._setupSPAMonitoring();
          }
        }, 500);
      }
    },
    
    // Detect debug mode from URL parameter
    _detectDebugMode: function() {
      const urlParams = new URLSearchParams(window.location.search);
      if (urlParams.has('debug')) {
        this._debugMode = true;
        console.log('[AnyWP] Debug mode ENABLED via URL parameter');
      }
    },
    
    // Enable debug mode manually
    enableDebug: function() {
      this._debugMode = true;
      console.log('[AnyWP] Debug mode ENABLED manually');
    },
    
    // Enable/Disable SPA mode manually
    setSPAMode: function(enabled) {
      this._spaMode = enabled;
      console.log('[AnyWP] SPA mode: ' + (enabled ? 'ENABLED' : 'DISABLED'));
      
      if (enabled) {
        this._setupSPAMonitoring();
      } else {
        this._teardownSPAMonitoring();
      }
    },
    
    // Log with debug control
    _log: function(message, forceLog) {
      if (this._debugMode || forceLog) {
        console.log('[AnyWP] ' + message);
      }
    },
    
    // Calculate element bounds in physical pixels
    _calculateElementBounds: function(element) {
      const rect = element.getBoundingClientRect();
      const dpi = this.dpiScale;
      
      return {
        left: Math.round(rect.left * dpi),
        top: Math.round(rect.top * dpi),
        right: Math.round(rect.right * dpi),
        bottom: Math.round(rect.bottom * dpi),
        width: Math.round(rect.width * dpi),
        height: Math.round(rect.height * dpi)
      };
    },
    
    // Show debug border
    _showDebugBorder: function(bounds, element) {
      // Remove old border if exists
      const oldBorder = element._anywpDebugBorder;
      if (oldBorder && oldBorder.parentNode) {
        oldBorder.parentNode.removeChild(oldBorder);
      }
      
      const border = document.createElement('div');
      border.className = 'AnyWP-debug-border';
      border.style.cssText = 
        'position: fixed;' +
        'left: ' + (bounds.left / this.dpiScale) + 'px;' +
        'top: ' + (bounds.top / this.dpiScale) + 'px;' +
        'width: ' + (bounds.width / this.dpiScale) + 'px;' +
        'height: ' + (bounds.height / this.dpiScale) + 'px;' +
        'border: 2px solid red;' +
        'box-shadow: 0 0 10px red;' +
        'pointer-events: none;' +
        'z-index: 999999;';
      document.body.appendChild(border);
      
      // Store reference for later removal
      element._anywpDebugBorder = border;
    },
    
    // Check if point is in bounds
    _isInBounds: function(x, y, bounds) {
      return x >= bounds.left && x <= bounds.right &&
             y >= bounds.top && y <= bounds.bottom;
    },
    
    // Handle click event from native
    _handleClick: function(x, y) {
      console.log('[AnyWP] _handleClick called with: (' + x + ',' + y + ')');
      console.log('[AnyWP] Registered handlers: ' + this._clickHandlers.length);
      
      this._log('Click at physical: (' + x + ',' + y + ') CSS: (' + 
                (x / this.dpiScale) + ',' + (y / this.dpiScale) + ')');
      
      for (let i = 0; i < this._clickHandlers.length; i++) {
        const handler = this._clickHandlers[i];
        console.log('[AnyWP] Checking handler ' + i + ':', handler.bounds);
        console.log('[AnyWP] Is in bounds?', this._isInBounds(x, y, handler.bounds));
        
        if (this._isInBounds(x, y, handler.bounds)) {
          console.log('[AnyWP] HIT! Calling callback');
          this._log('  -> HIT: ' + (handler.element.id || handler.element.className));
          handler.callback(x, y);
          break;
        }
      }
      
      console.log('[AnyWP] No handler matched the click');
    },
    
    // Wait for element to appear in DOM
    _waitForElement: function(selector, callback, maxWait) {
      const self = this;
      const startTime = Date.now();
      maxWait = maxWait || 10000; // Default 10 seconds
      
      function check() {
        const element = document.querySelector(selector);
        if (element) {
          callback(element);
        } else if (Date.now() - startTime < maxWait) {
          setTimeout(check, 100);
        } else {
          console.error('[AnyWP] Element not found after ' + maxWait + 'ms: ' + selector);
        }
      }
      
      check();
    },
    
    // Register click handler with SPA support
    onClick: function(element, callback, options) {
      const self = this;
      options = options || {};
      
      // Parse options
      const immediate = options.immediate || false;
      const waitFor = options.waitFor !== undefined ? options.waitFor : !immediate;
      const maxWait = options.maxWait || 10000;
      const autoRefresh = options.autoRefresh !== undefined ? options.autoRefresh : this._autoRefreshEnabled;
      const delay = options.delay || (immediate ? 0 : 100);
      
      function registerElement(el) {
        if (!el) {
          console.error('[AnyWP] Element not found:', element);
          return;
        }
        
        // Check if already registered
        const existingIndex = self._clickHandlers.findIndex(function(h) {
          return h.element === el;
        });
        
        if (existingIndex !== -1) {
          self._log('Element already registered, updating bounds...');
          self._clickHandlers.splice(existingIndex, 1);
        }
        
        // Calculate bounds
        const bounds = self._calculateElementBounds(el);
        
        // Register handler
        const handlerData = {
          element: el,
          callback: callback,
          bounds: bounds,
          selector: typeof element === 'string' ? element : null,
          autoRefresh: autoRefresh,
          options: options
        };
        
        self._clickHandlers.push(handlerData);
        
        // Setup ResizeObserver for this element
        if (autoRefresh && window.ResizeObserver) {
          const resizeObserver = new ResizeObserver(function() {
            self._refreshElementBounds(handlerData);
          });
          resizeObserver.observe(el);
          handlerData.resizeObserver = resizeObserver;
        }
        
        // Debug output
        const showDebug = (options.debug !== undefined) ? options.debug : self._debugMode;
        if (showDebug) {
          console.log('----------------------------------------');
          console.log('Click Handler Registered:');
          console.log('  Element:', el.id || el.className || el.tagName || 'unknown');
          console.log('  Physical: [' + bounds.left + ',' + bounds.top + '] ~ [' + 
                      bounds.right + ',' + bounds.bottom + ']');
          console.log('  Size: ' + bounds.width + 'x' + bounds.height + ' px');
          console.log('  CSS: [' + (bounds.left / self.dpiScale) + ',' + 
                      (bounds.top / self.dpiScale) + '] ' + 
                      Math.round(bounds.width / self.dpiScale) + 'x' + 
                      Math.round(bounds.height / self.dpiScale));
          console.log('  Mode: ' + (self._spaMode ? 'SPA' : 'Static'));
          console.log('  Auto-refresh: ' + autoRefresh);
          console.log('----------------------------------------');
          
          self._showDebugBorder(bounds, el);
        }
      }
      
      // Execute registration with appropriate strategy
      if (immediate) {
        // Immediate mode: register now
        let el = element;
        if (typeof element === 'string') {
          el = document.querySelector(element);
        }
        registerElement(el);
      } else if (waitFor && typeof element === 'string') {
        // Wait for element mode: for SPA
        this._waitForElement(element, registerElement, maxWait);
      } else {
        // Delay mode: traditional
        setTimeout(function() {
          let el = element;
          if (typeof element === 'string') {
            el = document.querySelector(element);
          }
          registerElement(el);
        }, delay);
      }
    },
    
    // Refresh bounds for a specific handler
    _refreshElementBounds: function(handler) {
      if (!handler.element || !handler.element.isConnected) {
        this._log('Element disconnected, skipping refresh');
        return;
      }
      
      const newBounds = this._calculateElementBounds(handler.element);
      
      // Check if bounds actually changed
      const changed = 
        newBounds.left !== handler.bounds.left ||
        newBounds.top !== handler.bounds.top ||
        newBounds.width !== handler.bounds.width ||
        newBounds.height !== handler.bounds.height;
      
      if (changed) {
        handler.bounds = newBounds;
        this._log('Bounds refreshed for: ' + (handler.element.id || handler.element.className));
        
        // Update debug border if exists
        if (handler.element._anywpDebugBorder) {
          this._showDebugBorder(newBounds, handler.element);
        }
      }
    },
    
    // Refresh all registered click handlers' bounds
    refreshBounds: function() {
      const self = this;
      let refreshed = 0;
      
      this._clickHandlers.forEach(function(handler) {
        if (handler.element && handler.element.isConnected) {
          self._refreshElementBounds(handler);
          refreshed++;
        }
      });
      
      this._log('Refreshed bounds for ' + refreshed + ' elements', true);
      return refreshed;
    },
    
    // Clear all registered handlers
    clearHandlers: function() {
      // Cleanup observers
      this._clickHandlers.forEach(function(handler) {
        if (handler.resizeObserver) {
          handler.resizeObserver.disconnect();
        }
        if (handler.element && handler.element._anywpDebugBorder) {
          const border = handler.element._anywpDebugBorder;
          if (border.parentNode) {
            border.parentNode.removeChild(border);
          }
        }
      });
      
      this._clickHandlers = [];
      this._log('All handlers cleared', true);
    },
    
    // Setup SPA monitoring (route changes, DOM mutations)
    _setupSPAMonitoring: function() {
      const self = this;
      
      // Monitor history changes (SPA routing)
      const originalPushState = history.pushState;
      const originalReplaceState = history.replaceState;
      
      history.pushState = function() {
        originalPushState.apply(history, arguments);
        self._onRouteChange();
      };
      
      history.replaceState = function() {
        originalReplaceState.apply(history, arguments);
        self._onRouteChange();
      };
      
      window.addEventListener('popstate', function() {
        self._onRouteChange();
      });
      
      // Monitor DOM mutations
      if (window.MutationObserver) {
        this._mutationObserver = new MutationObserver(function(mutations) {
          let shouldRefresh = false;
          
          mutations.forEach(function(mutation) {
            // Check if any registered element was affected
            if (mutation.type === 'childList' || mutation.type === 'attributes') {
              self._clickHandlers.forEach(function(handler) {
                if (mutation.target === handler.element || 
                    mutation.target.contains(handler.element)) {
                  shouldRefresh = true;
                }
              });
            }
          });
          
          if (shouldRefresh) {
            self.refreshBounds();
          }
        });
        
        this._mutationObserver.observe(document.body, {
          childList: true,
          subtree: true,
          attributes: true,
          attributeFilter: ['class', 'style']
        });
      }
      
      this._log('SPA monitoring enabled', true);
    },
    
    // Teardown SPA monitoring
    _teardownSPAMonitoring: function() {
      if (this._mutationObserver) {
        this._mutationObserver.disconnect();
        this._mutationObserver = null;
      }
      this._log('SPA monitoring disabled');
    },
    
    // Handle SPA route change
    _onRouteChange: function() {
      const self = this;
      this._log('Route changed, refreshing in 500ms...');
      
      // Delay to allow new content to render
      setTimeout(function() {
        // Re-register handlers with selectors
        self._clickHandlers.forEach(function(handler) {
          if (handler.selector && handler.autoRefresh) {
            // Try to find element again
            const newElement = document.querySelector(handler.selector);
            if (newElement && newElement !== handler.element) {
              self._log('Element re-mounted: ' + handler.selector);
              handler.element = newElement;
              self._refreshElementBounds(handler);
              
              // Re-setup ResizeObserver
              if (handler.resizeObserver) {
                handler.resizeObserver.disconnect();
              }
              if (window.ResizeObserver) {
                const resizeObserver = new ResizeObserver(function() {
                  self._refreshElementBounds(handler);
                });
                resizeObserver.observe(newElement);
                handler.resizeObserver = resizeObserver;
              }
            }
          }
        });
        
        self.refreshBounds();
      }, 500);
    },
    
    // Open URL in default browser
    openURL: function(url) {
      this._log('Opening URL: ' + url);
      
      // Call native method via postMessage
      if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
          type: 'openURL',
          url: url
        });
      } else {
        console.warn('[AnyWP] Native bridge not available, opening in current window');
        window.open(url, '_blank');
      }
    },
    
    // Notify wallpaper is ready
    ready: function(name) {
      this._log('Wallpaper ready: ' + name, true);
      
      if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
          type: 'ready',
          name: name
        });
      }
    },
    
    // Register mouse event callback
    onMouse: function(callback) {
      this._mouseCallbacks.push(callback);
      this._log('Mouse callback registered (total: ' + this._mouseCallbacks.length + ')');
    },
    
    // Register keyboard event callback
    onKeyboard: function(callback) {
      this._keyboardCallbacks.push(callback);
      this._log('Keyboard callback registered (total: ' + this._keyboardCallbacks.length + ')');
    },
    
    // Setup event listeners
    _setupEventListeners: function() {
      const self = this;
      
      // Listen for custom events from native
      window.addEventListener('AnyWP:mouse', function(event) {
        const detail = event.detail;
        self._mouseCallbacks.forEach(function(cb) {
          cb(detail);
        });
      });
      
      window.addEventListener('AnyWP:keyboard', function(event) {
        const detail = event.detail;
        self._keyboardCallbacks.forEach(function(cb) {
          cb(detail);
        });
      });
      
      window.addEventListener('AnyWP:click', function(event) {
        console.log('[AnyWP] AnyWP:click event received:', event.detail);
        const detail = event.detail;
        self._handleClick(detail.x, detail.y);
      });
      
      window.addEventListener('AnyWP:interactionMode', function(event) {
        self.interactionEnabled = event.detail.enabled;
        self._log('Interaction mode: ' + (self.interactionEnabled ? 'ON' : 'OFF'), true);
      });
      
      // Listen for window resize
      window.addEventListener('resize', function() {
        self._log('Window resized, refreshing bounds...');
        setTimeout(function() {
          self.refreshBounds();
        }, 200);
      });
    },
    
    // Helper: React useEffect integration
    useReactEffect: function(element, callback, options) {
      const self = this;
      return function() {
        // Register on mount
        self.onClick(element, callback, Object.assign({}, options, { immediate: true }));
        
        // Cleanup on unmount
        return function() {
          self._clickHandlers = self._clickHandlers.filter(function(h) {
            return h.element !== element;
          });
        };
      };
    },
    
    // Helper: Vue mounted/unmounted integration
    vueLifecycle: function(element, callback, options) {
      const self = this;
      return {
        mounted: function() {
          self.onClick(element, callback, Object.assign({}, options, { immediate: true }));
        },
        unmounted: function() {
          self._clickHandlers = self._clickHandlers.filter(function(h) {
            return h.element !== element;
          });
        }
      };
    }
  };
  
  // Auto initialize
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function() {
      AnyWP._init();
    });
  } else {
    AnyWP._init();
  }
  
  console.log('[AnyWP] SDK loaded successfully');
})();
