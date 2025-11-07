// Click handler module
import { Debug } from '../utils/debug.js';
import { Bounds } from '../utils/bounds.js';

export const ClickHandler = {
  // Handle click event from native
  handleClick(anyWP, x, y) {
    Debug.log('Click at: (' + x + ',' + y + ')');
    
    for (let i = 0; i < anyWP._clickHandlers.length; i++) {
      const handler = anyWP._clickHandlers[i];
      
      if (Bounds.isInBounds(x, y, handler.bounds)) {
        Debug.log('HIT: ' + (handler.element.id || handler.element.className));
        handler.callback(x, y);
        break;
      }
    }
  },
  
  // Wait for element to appear in DOM
  waitForElement(selector, callback, maxWait = 10000) {
    const startTime = Date.now();
    
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
  onClick(anyWP, element, callback, options = {}) {
    const self = this;
    const immediate = options.immediate || false;
    const waitFor = options.waitFor !== undefined ? options.waitFor : !immediate;
    const maxWait = options.maxWait || 10000;
    const autoRefresh = options.autoRefresh !== undefined ? options.autoRefresh : anyWP._autoRefreshEnabled;
    const delay = options.delay || (immediate ? 0 : 100);
    
    function registerElement(el) {
      if (!el) {
        console.error('[AnyWP] Element not found:', element);
        return;
      }
      
      // Check if already registered
      const existingIndex = anyWP._clickHandlers.findIndex(function(h) {
        return h.element === el;
      });
      
      if (existingIndex !== -1) {
        Debug.log('Element already registered, updating bounds...');
        anyWP._clickHandlers.splice(existingIndex, 1);
      }
      
      // Calculate bounds
      const bounds = Bounds.calculate(el, anyWP.dpiScale);
      
      // Register handler
      const handlerData = {
        element: el,
        callback: callback,
        bounds: bounds,
        selector: typeof element === 'string' ? element : null,
        autoRefresh: autoRefresh,
        options: options
      };
      
      anyWP._clickHandlers.push(handlerData);
      
      // Setup ResizeObserver
      if (autoRefresh && window.ResizeObserver) {
        const resizeObserver = new ResizeObserver(function() {
          self.refreshElementBounds(anyWP, handlerData);
        });
        resizeObserver.observe(el);
        handlerData.resizeObserver = resizeObserver;
      }
      
      // Debug output
      const showDebug = (options.debug !== undefined) ? options.debug : anyWP._debugMode;
      if (showDebug) {
        console.log('[AnyWP] Click Handler Registered:', el.id || el.className);
        console.log('  Physical: [' + bounds.left + ',' + bounds.top + '] ~ [' + 
                    bounds.right + ',' + bounds.bottom + ']');
        console.log('  Size: ' + bounds.width + 'x' + bounds.height);
        
        Debug.showBorder(bounds, el, anyWP.dpiScale);
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
      this.waitForElement(element, registerElement, maxWait);
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
  refreshElementBounds(anyWP, handler) {
    if (!handler.element || !handler.element.isConnected) {
      return;
    }
    
    const newBounds = Bounds.calculate(handler.element, anyWP.dpiScale);
    
    const changed = 
      newBounds.left !== handler.bounds.left ||
      newBounds.top !== handler.bounds.top ||
      newBounds.width !== handler.bounds.width ||
      newBounds.height !== handler.bounds.height;
    
    if (changed) {
      handler.bounds = newBounds;
      
      if (handler.element._anywpDebugBorder) {
        Debug.showBorder(newBounds, handler.element, anyWP.dpiScale);
      }
    }
  },
  
  // Refresh all registered click handlers' bounds
  refreshBounds(anyWP) {
    const self = this;
    let refreshed = 0;
    
    anyWP._clickHandlers.forEach(function(handler) {
      if (handler.element && handler.element.isConnected) {
        self.refreshElementBounds(anyWP, handler);
        refreshed++;
      }
    });
    
    Debug.log('Refreshed ' + refreshed + ' elements', true);
    return refreshed;
  },
  
  // Clear all registered handlers
  clearHandlers(anyWP) {
    anyWP._clickHandlers.forEach(function(handler) {
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
    
    anyWP._clickHandlers = [];
    Debug.log('All handlers cleared', true);
  }
};

