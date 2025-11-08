// Click handler module
import { Debug } from '../utils/debug';
import { Bounds } from '../utils/bounds';
import type { AnyWPSDK, ClickCallback, ClickHandlerOptions, ClickHandlerData } from '../types';

declare global {
  interface HTMLElement {
    _anywpDebugBorder?: HTMLElement;
  }
}

export const ClickHandler = {
  // Handle click event from native
  handleClick(x: number, y: number) {
    Debug.log('Click at: (' + x + ',' + y + ')');
    
    const anyWP = (window as any).AnyWP as AnyWPSDK;
    
    for (let i = 0; i < anyWP._clickHandlers.length; i++) {
      const handler = anyWP._clickHandlers[i];
      
      if (handler && Bounds.isInBounds(x, y, handler.bounds)) {
        Debug.log('HIT: ' + (handler.element.id || handler.element.className));
        handler.callback(x, y);
        break;
      }
    }
  },
  
  // Wait for element to appear in DOM
  waitForElement(selector: string, callback: (el: HTMLElement) => void, maxWait: number = 10000) {
    const startTime = Date.now();
    
    function check() {
      const element = document.querySelector(selector) as HTMLElement | null;
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
  onClick(anyWP: AnyWPSDK, element: string | HTMLElement, callback: ClickCallback, options: ClickHandlerOptions = {}) {
    // Validate parameters
    if (typeof element === 'function') {
      console.error('[AnyWP] onClick: First parameter should be element, not callback!');
      console.error('[AnyWP] Correct usage: AnyWP.onClick(element, callback, options)');
      console.error('[AnyWP] Example: AnyWP.onClick("#my-button", (x, y) => { ... })');
      throw new TypeError('[AnyWP] onClick: Invalid parameters - element must be HTMLElement or selector string');
    }
    
    if (typeof callback !== 'function') {
      console.error('[AnyWP] onClick: Second parameter must be a callback function!');
      console.error('[AnyWP] Correct usage: AnyWP.onClick(element, callback, options)');
      throw new TypeError('[AnyWP] onClick: callback must be a function');
    }
    
    const self = this;
    const immediate = options.immediate || false;
    const waitFor = options.waitFor !== undefined ? options.waitFor : !immediate;
    const maxWait = options.maxWait || 10000;
    const autoRefresh = options.autoRefresh !== undefined ? options.autoRefresh : anyWP._autoRefreshEnabled;
    const delay = options.delay || (immediate ? 0 : 100);
    
    function registerElement(el: HTMLElement | null) {
      if (!el) {
        console.error('[AnyWP] Element not found:', element);
        return;
      }
      
      // Check if already registered (prevent duplicate handlers)
      const existingIndex = anyWP._clickHandlers.findIndex(function(h) {
        return h.element === el;
      });
      
      if (existingIndex !== -1) {
        const existingHandler = anyWP._clickHandlers[existingIndex];
        if (existingHandler) {
          console.warn('[AnyWP] Element already has click handler, skipping duplicate registration:', el.id || el.className);
          console.warn('[AnyWP] Existing handler callback:', existingHandler.callback.toString().substring(0, 100));
          console.warn('[AnyWP] New callback (ignored):', callback.toString().substring(0, 100));
        }
        return; // Skip duplicate registration instead of replacing
      }
      
      // Calculate bounds
      const bounds = Bounds.calculate(el, anyWP.dpiScale);
      
      // Register handler
      const handlerData: ClickHandlerData = {
        element: el,
        callback: callback,
        bounds: bounds,
        selector: typeof element === 'string' ? element : null,
        autoRefresh: autoRefresh,
        options: options
      };
      
      anyWP._clickHandlers.push(handlerData);
      
      // ========== Auto-Refresh: 智能位置跟踪 ==========
      if (autoRefresh) {
        // 方案 1: ResizeObserver - 监听元素自身大小变化（快速响应）
        if (window.ResizeObserver) {
          const resizeObserver = new ResizeObserver(function() {
            self.refreshElementBounds(anyWP, handlerData);
          });
          resizeObserver.observe(el);
          handlerData.resizeObserver = resizeObserver;
        }
        
        // 方案 2: IntersectionObserver - 监听元素位置/可见性变化（高效）
        if (window.IntersectionObserver) {
          const intersectionObserver = new IntersectionObserver(function(entries) {
            entries.forEach(function(entry) {
              // 元素位置变化或可见性变化时触发
              if (entry.isIntersecting || entry.intersectionRatio > 0) {
                self.refreshElementBounds(anyWP, handlerData);
              }
            });
          }, {
            threshold: [0, 0.1, 0.5, 0.9, 1.0]  // 多个阈值，提高灵敏度
          });
          intersectionObserver.observe(el);
          handlerData.intersectionObserver = intersectionObserver;
        }
        
        // 方案 3: 定期检查位置变化（兜底方案，适用于复杂布局）
        if (!handlerData.positionCheckTimer) {
          handlerData.lastBounds = bounds;
          handlerData.positionCheckTimer = setInterval(function() {
            if (!el.isConnected) {
              clearInterval(handlerData.positionCheckTimer);
              return;
            }
            
            // 检查位置是否变化
            const rect = el.getBoundingClientRect();
            const currentLeft = Math.round(rect.left * anyWP.dpiScale);
            const currentTop = Math.round(rect.top * anyWP.dpiScale);
            
            if (handlerData.lastBounds && 
                (currentLeft !== handlerData.lastBounds.left || 
                 currentTop !== handlerData.lastBounds.top)) {
              // 位置变化，刷新
              self.refreshElementBounds(anyWP, handlerData);
              Debug.log('Position changed for: ' + (el.id || el.className), false);
            }
          }, 500);  // 每 500ms 检查一次，性能开销低
        }
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
      let el: HTMLElement | null;
      if (typeof element === 'string') {
        el = document.querySelector(element) as HTMLElement | null;
      } else {
        el = element;
      }
      registerElement(el);
    } else if (waitFor && typeof element === 'string') {
      this.waitForElement(element, registerElement, maxWait);
    } else {
      setTimeout(function() {
        let el: HTMLElement | null;
        if (typeof element === 'string') {
          el = document.querySelector(element) as HTMLElement | null;
        } else {
          el = element;
        }
        registerElement(el);
      }, delay);
    }
  },
  
  // Refresh bounds for a specific handler
  refreshElementBounds(anyWP: AnyWPSDK, handler: ClickHandlerData) {
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
      handler.lastBounds = newBounds;  // 更新最后已知位置
      
      if (handler.element._anywpDebugBorder) {
        Debug.showBorder(newBounds, handler.element, anyWP.dpiScale);
      }
    }
  },
  
  // Refresh all registered click handlers' bounds
  refreshBounds(anyWP: AnyWPSDK): number {
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
  clearHandlers(anyWP: AnyWPSDK) {
    anyWP._clickHandlers.forEach(function(handler) {
      // 清理 ResizeObserver
      if (handler.resizeObserver) {
        handler.resizeObserver.disconnect();
      }
      
      // 清理 IntersectionObserver
      if (handler.intersectionObserver) {
        handler.intersectionObserver.disconnect();
      }
      
      // 清理定时器
      if (handler.positionCheckTimer) {
        clearInterval(handler.positionCheckTimer);
      }
      
      // 清理调试边框
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

