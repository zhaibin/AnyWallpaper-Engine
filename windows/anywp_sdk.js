// AnyWP Engine SDK v4.0.0 - JavaScript Bridge
// Auto-injected into WebView2
// React/Vue SPA Compatible

(function() {
  'use strict';

  // AnyWP Global Object
  window.AnyWP = {
    version: '4.2.0',
    dpiScale: window.devicePixelRatio || 1,
    screenWidth: screen.width * (window.devicePixelRatio || 1),
    screenHeight: screen.height * (window.devicePixelRatio || 1),
    interactionEnabled: true,  // Default: enabled for drag support
    
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
    
    // Initialize
    _init: function() {
      console.log('========================================');
      console.log('AnyWP Engine v' + this.version + ' (SPA Compatible)');
      console.log('========================================');
      console.log('Screen: ' + this.screenWidth + 'x' + this.screenHeight);
      console.log('DPI Scale: ' + this.dpiScale + 'x');
      console.log('Interaction Enabled: ' + this.interactionEnabled);
      console.log('========================================');
      
      this._detectDebugMode();
      this._detectSPA();
      this._setupEventListeners();
      
      // Enable debug mode automatically for testing
      this._debugMode = true;
      console.log('[AnyWP] Debug mode ENABLED automatically');
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
        console.log('[AnyWP] SPA Framework detected: ' + framework);
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
            console.log('[AnyWP] SPA Framework detected: ' + framework);
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
        console.log('[AnyWP] Debug mode ENABLED');
      }
    },
    
    // Enable debug mode manually
    enableDebug: function() {
      this._debugMode = true;
      console.log('[AnyWP] Debug mode ENABLED');
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
      
      element._anywpDebugBorder = border;
    },
    
    // Check if point is in bounds
    _isInBounds: function(x, y, bounds) {
      return x >= bounds.left && x <= bounds.right &&
             y >= bounds.top && y <= bounds.bottom;
    },
    
    // Check if mouse (in physical pixels) is over element
    _isMouseOverElement: function(mouseX, mouseY, element) {
      const rect = element.getBoundingClientRect();
      const dpi = this.dpiScale;
      
      const physicalLeft = Math.round(rect.left * dpi);
      const physicalTop = Math.round(rect.top * dpi);
      const physicalRight = Math.round(rect.right * dpi);
      const physicalBottom = Math.round(rect.bottom * dpi);
      
      return mouseX >= physicalLeft && mouseX <= physicalRight &&
             mouseY >= physicalTop && mouseY <= physicalBottom;
    },
    
    // Handle click event from native
    _handleClick: function(x, y) {
      this._log('Click at: (' + x + ',' + y + ')');
      
      for (let i = 0; i < this._clickHandlers.length; i++) {
        const handler = this._clickHandlers[i];
        
        if (this._isInBounds(x, y, handler.bounds)) {
          this._log('HIT: ' + (handler.element.id || handler.element.className));
          handler.callback(x, y);
          break;
        }
      }
    },
    
    // Wait for element to appear in DOM
    _waitForElement: function(selector, callback, maxWait) {
      const self = this;
      const startTime = Date.now();
      maxWait = maxWait || 10000;
      
      function check() {
        const element = document.querySelector(selector);
        if (element) {
          callback(element);
        } else if (Date.now() - startTime < maxWait) {
          setTimeout(check, 100);
        } else {
          console.error('[AnyWP] Element not found: ' + selector);
        }
      }
      
      check();
    },
    
    // Register click handler with SPA support
    onClick: function(element, callback, options) {
      const self = this;
      options = options || {};
      
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
        
        // Setup ResizeObserver
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
          console.log('[AnyWP] Click Handler Registered:', el.id || el.className);
          console.log('  Physical: [' + bounds.left + ',' + bounds.top + '] ~ [' + 
                      bounds.right + ',' + bounds.bottom + ']');
          console.log('  Size: ' + bounds.width + 'x' + bounds.height);
          
          self._showDebugBorder(bounds, el);
        }
      }
      
      // Execute registration
      if (immediate) {
        let el = element;
        if (typeof element === 'string') {
          el = document.querySelector(element);
        }
        registerElement(el);
      } else if (waitFor && typeof element === 'string') {
        this._waitForElement(element, registerElement, maxWait);
      } else {
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
        return;
      }
      
      const newBounds = this._calculateElementBounds(handler.element);
      
      const changed = 
        newBounds.left !== handler.bounds.left ||
        newBounds.top !== handler.bounds.top ||
        newBounds.width !== handler.bounds.width ||
        newBounds.height !== handler.bounds.height;
      
      if (changed) {
        handler.bounds = newBounds;
        
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
      
      this._log('Refreshed ' + refreshed + ' elements', true);
      return refreshed;
    },
    
    // Clear all registered handlers
    clearHandlers: function() {
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
    
    // Setup SPA monitoring
    _setupSPAMonitoring: function() {
      const self = this;
      
      // Monitor history changes
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
    },
    
    // Teardown SPA monitoring
    _teardownSPAMonitoring: function() {
      if (this._mutationObserver) {
        this._mutationObserver.disconnect();
        this._mutationObserver = null;
      }
    },
    
    // Handle SPA route change
    _onRouteChange: function() {
      const self = this;
      this._log('Route changed, refreshing...');
      
      setTimeout(function() {
        self._clickHandlers.forEach(function(handler) {
          if (handler.selector && handler.autoRefresh) {
            const newElement = document.querySelector(handler.selector);
            if (newElement && newElement !== handler.element) {
              handler.element = newElement;
              self._refreshElementBounds(handler);
              
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
      
      if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
          type: 'openURL',
          url: url
        });
      } else {
        console.warn('[AnyWP] Native bridge not available');
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
      this._log('Mouse callback registered');
    },
    
    // Register keyboard event callback
    onKeyboard: function(callback) {
      this._keyboardCallbacks.push(callback);
      this._log('Keyboard callback registered');
    },
    
  // Setup event listeners
  _setupEventListeners: function() {
    const self = this;
    
    window.addEventListener('AnyWP:mouse', function(event) {
      self._mouseCallbacks.forEach(function(cb) {
        cb(event.detail);
      });
    });
    
    window.addEventListener('AnyWP:keyboard', function(event) {
      self._keyboardCallbacks.forEach(function(cb) {
        cb(event.detail);
      });
    });
    
    window.addEventListener('AnyWP:click', function(event) {
      self._handleClick(event.detail.x, event.detail.y);
    });
    
    window.addEventListener('AnyWP:interactionMode', function(event) {
      self.interactionEnabled = event.detail.enabled;
      self._log('Interaction mode: ' + (self.interactionEnabled ? 'ON' : 'OFF'), true);
    });
    
    // NEW: Handle visibility changes for power saving
    window.addEventListener('AnyWP:visibility', function(event) {
      const visible = event.detail.visible;
      self._log('Visibility changed: ' + (visible ? 'visible' : 'hidden'), true);
      
      // Notify user callback if set
      if (self._visibilityCallback) {
        self._visibilityCallback(visible);
      }
      
      // Auto-pause animations when hidden (for better power saving)
      if (!visible) {
        self._autoPauseAnimations();
      } else {
        self._autoResumeAnimations();
      }
    });
    
    window.addEventListener('resize', function() {
      self._log('Window resized, refreshing...');
      setTimeout(function() {
        self.refreshBounds();
      }, 200);
    });
  },
  
  // NEW: Register visibility callback
  onVisibilityChange: function(callback) {
    this._visibilityCallback = callback;
    this._log('Visibility callback registered');
  },
  
  // NEW: Auto-pause animations when hidden
  _autoPauseAnimations: function() {
    if (!window.__anyWP_animationsPaused) {
      this._log('Auto-pausing animations for power saving');
      window.__anyWP_animationsPaused = true;
      
      // Pause videos
      const videos = document.querySelectorAll('video');
      videos.forEach(function(video) {
        if (!video.paused) {
          video.__anyWP_wasPlaying = true;
          video.pause();
        }
      });
      
      // Pause audio
      const audios = document.querySelectorAll('audio');
      audios.forEach(function(audio) {
        if (!audio.paused) {
          audio.__anyWP_wasPlaying = true;
          audio.pause();
        }
      });
    }
  },
  
  // NEW: Auto-resume animations when visible
  _autoResumeAnimations: function() {
    if (window.__anyWP_animationsPaused) {
      this._log('Auto-resuming animations');
      window.__anyWP_animationsPaused = false;
      
      // Resume videos
      const videos = document.querySelectorAll('video');
      videos.forEach(function(video) {
        if (video.__anyWP_wasPlaying) {
          video.play();
          delete video.__anyWP_wasPlaying;
        }
      });
      
      // Resume audio
      const audios = document.querySelectorAll('audio');
      audios.forEach(function(audio) {
        if (audio.__anyWP_wasPlaying) {
          audio.play();
          delete audio.__anyWP_wasPlaying;
        }
      });
    }
  },
  
  // ========== Drag & Drop Support ==========
  
  // Make element draggable
  makeDraggable: function(element, options) {
    const self = this;
    options = options || {};
    
    const persistKey = options.persistKey || null;
    const onDragStart = options.onDragStart || null;
    const onDrag = options.onDrag || null;
    const onDragEnd = options.onDragEnd || null;
    const bounds = options.bounds || null; // {left, top, right, bottom}
    
    let el = element;
    if (typeof element === 'string') {
      el = document.querySelector(element);
    }
    
    if (!el) {
      console.error('[AnyWP] Element not found:', element);
      return;
    }
    
    // Load persisted position from storage
    if (persistKey) {
      self.loadState(persistKey, function(savedPos) {
        if (savedPos) {
          el.style.position = 'absolute';
          el.style.left = savedPos.left + 'px';
          el.style.top = savedPos.top + 'px';
          console.log('[AnyWP] Restored position for ' + persistKey + ': ' + savedPos.left + ',' + savedPos.top);
        }
      });
    }
    
    // Register draggable element
    const draggableData = {
      element: el,
      persistKey: persistKey,
      onDragStart: onDragStart,
      onDrag: onDrag,
      onDragEnd: onDragEnd,
      bounds: bounds
    };
    
    self._draggableElements.push(draggableData);
    
    // Make element absolutely positioned if not already
    if (window.getComputedStyle(el).position === 'static') {
      el.style.position = 'absolute';
    }
    
    // Set cursor and disable system drag behaviors
    el.style.cursor = 'move';
    el.style.userSelect = 'none';
    el.style.webkitUserSelect = 'none';
    el.style.mozUserSelect = 'none';
    el.style.msUserSelect = 'none';
    
    // Prevent system drag/drop
    el.draggable = false;
    el.ondragstart = function(e) { 
      e.preventDefault(); 
      return false; 
    };
    
    // Prevent text selection during drag
    el.onselectstart = function(e) {
      e.preventDefault();
      return false;
    };
    
    // IMPORTANT: Use AnyWP:mouse events instead of DOM events
    // This allows dragging even with transparent windows (mouse hook architecture)
    
    // Register global mouse handler for this element
    function handleGlobalMouse(event) {
      if (!self.interactionEnabled) {
        self._log('[makeDraggable] Interaction disabled, ignoring event');
        return;
      }
      
      const detail = event.detail;
      const mouseX = detail.x;
      const mouseY = detail.y;
      const mouseType = detail.type;
      
      // Debug log for non-mousemove events
      if (mouseType !== 'mousemove') {
        self._log('[makeDraggable] Mouse event: ' + mouseType + ' at (' + mouseX + ',' + mouseY + ')');
      }
      
      // Check if mouse is over this element (reuse common function)
      const isOver = self._isMouseOverElement(mouseX, mouseY, el);
      
      // Debug: log mousedown events
      if (mouseType === 'mousedown') {
        console.log('[AnyWP] [makeDraggable] mousedown - isOver:', isOver, 
                    'dragState:', self._dragState ? 'EXISTS' : 'NULL');
      }
      
      if (mouseType === 'mousedown' && isOver && !self._dragState) {
        // Start dragging
        console.log('[AnyWP] [Drag] START - Mouse at (' + mouseX + ',' + mouseY + ')');
        
        // Get element bounds for offset calculation
        const rect = el.getBoundingClientRect();
        const dpi = self.dpiScale;
        const physicalLeft = Math.round(rect.left * dpi);
        const physicalTop = Math.round(rect.top * dpi);
        
        self._dragState = {
          element: el,
          data: draggableData,
          startX: mouseX,
          startY: mouseY,
          offsetX: mouseX - physicalLeft,
          offsetY: mouseY - physicalTop,
          initialLeft: physicalLeft,
          initialTop: physicalTop
        };
        
        if (onDragStart) {
          onDragStart({
            x: physicalLeft / dpi,
            y: physicalTop / dpi
          });
        }
        
        self._log('Drag start at: ' + mouseX + ',' + mouseY + ' (element at ' + physicalLeft + ',' + physicalTop + ')', true);
      }
      else if (mouseType === 'mousemove' && self._dragState && self._dragState.element === el) {
        // Continue dragging
        console.log('[AnyWP] [Drag] MOVE - Mouse:', mouseX, mouseY);
        
        let newPhysicalLeft = mouseX - self._dragState.offsetX;
        let newPhysicalTop = mouseY - self._dragState.offsetY;
        
        // Apply bounds if specified (convert to physical pixels)
        if (bounds) {
          if (bounds.left !== undefined) {
            newPhysicalLeft = Math.max(bounds.left * dpi, newPhysicalLeft);
          }
          if (bounds.top !== undefined) {
            newPhysicalTop = Math.max(bounds.top * dpi, newPhysicalTop);
          }
          if (bounds.right !== undefined) {
            newPhysicalLeft = Math.min((bounds.right - el.offsetWidth) * dpi, newPhysicalLeft);
          }
          if (bounds.bottom !== undefined) {
            newPhysicalTop = Math.min((bounds.bottom - el.offsetHeight) * dpi, newPhysicalTop);
          }
        }
        
        // Convert back to CSS pixels
        const newLeft = newPhysicalLeft / dpi;
        const newTop = newPhysicalTop / dpi;
        
        el.style.left = newLeft + 'px';
        el.style.top = newTop + 'px';
        
        if (onDrag) {
          onDrag({
            x: newLeft,
            y: newTop,
            deltaX: (mouseX - self._dragState.startX) / dpi,
            deltaY: (mouseY - self._dragState.startY) / dpi
          });
        }
      }
      else if (mouseType === 'mouseup' && self._dragState && self._dragState.element === el) {
        // End dragging
        console.log('[AnyWP] [Drag] END');
        
        const finalRect = el.getBoundingClientRect();
        const finalPos = {
          x: finalRect.left,
          y: finalRect.top
        };
        
        // Save position if persistKey is provided
        if (persistKey) {
          self._saveElementPosition(persistKey, finalPos.x, finalPos.y);
        }
        
        if (onDragEnd) {
          onDragEnd(finalPos);
        }
        
        self._log('Drag end at: ' + finalPos.x + ',' + finalPos.y, true);
        
        self._dragState = null;
      }
      else if (mouseType === 'mousedown' && !isOver) {
        // Debug: mouse down but not over element (don't log - too noisy)
      }
      else if (mouseType === 'mousemove' && self._dragState && self._dragState.element !== el) {
        // Debug: moving but not this element
        console.log('[AnyWP] [makeDraggable] mousemove but wrong element');
      }
      else if (mouseType === 'mousemove' && !self._dragState) {
        // Debug: moving but no drag state
        // Don't log this - too noisy
      }
    }
    
    // Listen to global mouse events from native layer
    window.addEventListener('AnyWP:mouse', handleGlobalMouse);
    
    // Store event handler for cleanup
    el.__anyWP_dragHandler = {
      handleGlobalMouse: handleGlobalMouse
    };
    
    this._log('Element made draggable (via mouse hook): ' + (el.id || el.className));
  },
  
  // Remove draggable functionality
  removeDraggable: function(element) {
    let el = element;
    if (typeof element === 'string') {
      el = document.querySelector(element);
    }
    
    if (!el) {
      console.error('[AnyWP] Element not found:', element);
      return;
    }
    
    // Remove event handlers
    if (el.__anyWP_dragHandler) {
      window.removeEventListener('AnyWP:mouse', el.__anyWP_dragHandler.handleGlobalMouse);
      delete el.__anyWP_dragHandler;
    }
    
    // Remove from registry
    this._draggableElements = this._draggableElements.filter(function(d) {
      return d.element !== el;
    });
    
    // Reset cursor
    el.style.cursor = '';
    el.style.userSelect = '';
    
    this._log('Removed draggable from element: ' + (el.id || el.className));
  },
  
  // Save element position to native storage
  _saveElementPosition: function(key, x, y) {
    const self = this;
    const position = { left: x, top: y };
    
    console.log('[AnyWP] Saving position for ' + key + ': ', position);
    
    // Update local cache
    self._persistedState[key] = position;
    
    // Send to native layer
    if (window.chrome && window.chrome.webview) {
      const msg = {
        type: 'saveState',
        key: key,
        value: JSON.stringify(position)
      };
      console.log('[AnyWP] Sending saveState message:', msg);
      window.chrome.webview.postMessage(msg);
      console.log('[AnyWP] Message sent successfully');
    } else {
      console.warn('[AnyWP] chrome.webview not available, using localStorage');
      // Fallback to localStorage
      try {
        localStorage.setItem('AnyWP_' + key, JSON.stringify(position));
        console.log('[AnyWP] Saved to localStorage for ' + key);
      } catch (e) {
        console.error('[AnyWP] Failed to save state:', e);
      }
    }
  },
  
  // Load state from native storage
  loadState: function(key, callback) {
    const self = this;
    
    console.log('[AnyWP] Loading state for key:', key);
    
    // Check local cache first
    if (self._persistedState[key]) {
      console.log('[AnyWP] Found in cache:', self._persistedState[key]);
      callback(self._persistedState[key]);
      return;
    }
    
    // Request from native layer
    if (window.chrome && window.chrome.webview) {
      console.log('[AnyWP] Requesting state from native layer...');
      
      // Register one-time callback
      const handler = function(event) {
        if (event.detail && event.detail.type === 'stateLoaded' && event.detail.key === key) {
          console.log('[AnyWP] Received stateLoaded event:', event.detail);
          window.removeEventListener('AnyWP:stateLoaded', handler);
          
          const value = event.detail.value ? JSON.parse(event.detail.value) : null;
          self._persistedState[key] = value;
          console.log('[AnyWP] State loaded successfully:', value);
          callback(value);
        }
      };
      
      window.addEventListener('AnyWP:stateLoaded', handler);
      
      window.chrome.webview.postMessage({
        type: 'loadState',
        key: key
      });
      
      // Timeout after 1 second
      setTimeout(function() {
        window.removeEventListener('AnyWP:stateLoaded', handler);
        console.log('[AnyWP] loadState timeout for key:', key);
        callback(null);
      }, 1000);
    } else {
      console.warn('[AnyWP] chrome.webview not available, using localStorage');
      // Fallback to localStorage
      try {
        const stored = localStorage.getItem('AnyWP_' + key);
        const value = stored ? JSON.parse(stored) : null;
        self._persistedState[key] = value;
        console.log('[AnyWP] Loaded from localStorage:', value);
        callback(value);
      } catch (e) {
        console.error('[AnyWP] Failed to load state:', e);
        callback(null);
      }
    }
  },
  
  // Save custom state
  saveState: function(key, value) {
    const self = this;
    
    // Update local cache
    self._persistedState[key] = value;
    
    // Send to native layer
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage({
        type: 'saveState',
        key: key,
        value: JSON.stringify(value)
      });
      
      self._log('Saved state for ' + key);
    } else {
      // Fallback to localStorage
      try {
        localStorage.setItem('AnyWP_' + key, JSON.stringify(value));
        self._log('Saved state to localStorage for ' + key);
      } catch (e) {
        console.warn('[AnyWP] Failed to save state:', e);
      }
    }
  },
  
  // Clear all saved state
  clearState: function() {
    const self = this;
    
    self._persistedState = {};
    
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage({
        type: 'clearState'
      });
      
      self._log('Cleared all saved state');
    } else {
      // Clear localStorage
      try {
        const keys = Object.keys(localStorage);
        keys.forEach(function(key) {
          if (key.startsWith('AnyWP_')) {
            localStorage.removeItem(key);
          }
        });
        self._log('Cleared localStorage state');
      } catch (e) {
        console.warn('[AnyWP] Failed to clear state:', e);
      }
    }
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
