// Drag & Drop module
import { Debug } from '../utils/debug.js';
import { Coordinates } from '../utils/coordinates.js';
import { Storage } from './storage.js';

export const DragHandler = {
  // Make element draggable
  makeDraggable(anyWP, element, options = {}) {
    const self = this;
    const persistKey = options.persistKey || null;
    const onDragStart = options.onDragStart || null;
    const onDrag = options.onDrag || null;
    const onDragEnd = options.onDragEnd || null;
    const bounds = options.bounds || null;
    
    let el = element;
    if (typeof element === 'string') {
      el = document.querySelector(element);
    }
    
    if (!el) {
      console.error('[AnyWP] Element not found:', element);
      return;
    }
    
    // Load persisted position
    if (persistKey) {
      Storage.load(anyWP, persistKey, function(savedPos) {
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
    
    anyWP._draggableElements.push(draggableData);
    
    // Setup element styles
    if (window.getComputedStyle(el).position === 'static') {
      el.style.position = 'absolute';
    }
    
    el.style.cursor = 'move';
    el.style.userSelect = 'none';
    el.style.webkitUserSelect = 'none';
    el.style.mozUserSelect = 'none';
    el.style.msUserSelect = 'none';
    
    el.draggable = false;
    el.ondragstart = function(e) { 
      e.preventDefault(); 
      return false; 
    };
    
    el.onselectstart = function(e) {
      e.preventDefault();
      return false;
    };
    
    // Register global mouse handler
    function handleGlobalMouse(event) {
      if (!anyWP.interactionEnabled) {
        Debug.log('[makeDraggable] Interaction disabled, ignoring event');
        return;
      }
      
      const detail = event.detail;
      const mouseX = detail.x;
      const mouseY = detail.y;
      const mouseType = detail.type;
      
      const rect = el.getBoundingClientRect();
      const dpi = anyWP.dpiScale;
      
      const windowLeft = (typeof window.screenX !== 'undefined') ? window.screenX : 0;
      const windowTop = (typeof window.screenY !== 'undefined') ? window.screenY : 0;
      const docRect = document.documentElement.getBoundingClientRect();
      
      const viewportMouseX = (mouseX / dpi) - windowLeft - docRect.left;
      const viewportMouseY = (mouseY / dpi) - windowTop - docRect.top;
      
      const elementLeft = rect.left;
      const elementTop = rect.top;
      const elementRight = rect.right;
      const elementBottom = rect.bottom;
      
      const isOver = viewportMouseX >= elementLeft && viewportMouseX <= elementRight &&
                     viewportMouseY >= elementTop && viewportMouseY <= elementBottom;
      
      if (mouseType === 'mousedown') {
        const elementId = el.id || el.className || 'unknown';
        console.log('[AnyWP] [makeDraggable] mousedown on ' + elementId + ' - Mouse (screen):', mouseX, mouseY,
                    'Mouse (viewport CSS):', viewportMouseX.toFixed(1), viewportMouseY.toFixed(1),
                    'Element (viewport CSS):', elementLeft.toFixed(1), elementTop.toFixed(1), elementRight.toFixed(1), elementBottom.toFixed(1),
                    'isOver:', isOver,
                    'dragState:', anyWP._dragState ? 'EXISTS' : 'NULL',
                    'Window offset:', windowLeft, windowTop,
                    'Doc offset:', docRect.left.toFixed(1), docRect.top.toFixed(1),
                    'DPI:', dpi);
        
        const statusEl = document.getElementById('drag-status');
        if (statusEl) {
          if (isOver) {
            statusEl.innerHTML = '📍 检测到点击在 ' + elementId + ' 上 - 准备拖拽';
            statusEl.style.color = '#4CAF50';
          } else {
            statusEl.innerHTML = 
              '📍 点击不在 ' + elementId + ' 上 (鼠标: ' + viewportMouseX.toFixed(0) + ',' + viewportMouseY.toFixed(0) + 
              ' | 元素: ' + elementLeft.toFixed(0) + ',' + elementTop.toFixed(0) + '-' + elementRight.toFixed(0) + ',' + elementBottom.toFixed(0) + ')';
            statusEl.style.color = '#ff9800';
          }
        }
      }
      
      if (mouseType !== 'mousemove') {
        Debug.log('[makeDraggable] Mouse event: ' + mouseType + ' at screen (' + mouseX + ',' + mouseY + ') -> viewport CSS (' + viewportMouseX.toFixed(1) + ',' + viewportMouseY.toFixed(1) + ')');
      }
      
      if (mouseType === 'mousedown' && isOver && !anyWP._dragState) {
        console.log('[AnyWP] [Drag] START - Mouse at viewport CSS (' + viewportMouseX.toFixed(1) + ',' + viewportMouseY.toFixed(1) + ')');
        
        anyWP._dragState = {
          element: el,
          data: draggableData,
          startX: viewportMouseX,
          startY: viewportMouseY,
          offsetX: viewportMouseX - elementLeft,
          offsetY: viewportMouseY - elementTop,
          initialLeft: elementLeft,
          initialTop: elementTop,
          windowLeft: windowLeft,
          windowTop: windowTop,
          docLeft: docRect.left,
          docTop: docRect.top,
          dpi: dpi
        };
        
        if (onDragStart) {
          onDragStart({
            x: elementLeft,
            y: elementTop
          });
        }
        
        Debug.log('Drag start at: ' + viewportMouseX.toFixed(1) + ',' + viewportMouseY.toFixed(1) + ' (element at ' + elementLeft.toFixed(1) + ',' + elementTop.toFixed(1) + ')', true);
      }
      else if (mouseType === 'mousemove' && anyWP._dragState && anyWP._dragState.element === el) {
        const currentViewportX = (mouseX / anyWP._dragState.dpi) - anyWP._dragState.windowLeft - anyWP._dragState.docLeft;
        const currentViewportY = (mouseY / anyWP._dragState.dpi) - anyWP._dragState.windowTop - anyWP._dragState.docTop;
        
        let newLeft = currentViewportX - anyWP._dragState.offsetX;
        let newTop = currentViewportY - anyWP._dragState.offsetY;
        
        if (bounds) {
          if (bounds.left !== undefined) {
            newLeft = Math.max(bounds.left, newLeft);
          }
          if (bounds.top !== undefined) {
            newTop = Math.max(bounds.top, newTop);
          }
          if (bounds.right !== undefined) {
            newLeft = Math.min(bounds.right - el.offsetWidth, newLeft);
          }
          if (bounds.bottom !== undefined) {
            newTop = Math.min(bounds.bottom - el.offsetHeight, newTop);
          }
        }
        
        el.style.left = newLeft + 'px';
        el.style.top = newTop + 'px';
        
        if (onDrag) {
          onDrag({
            x: newLeft,
            y: newTop,
            deltaX: currentViewportX - anyWP._dragState.startX,
            deltaY: currentViewportY - anyWP._dragState.startY
          });
        }
      }
      else if (mouseType === 'mouseup' && anyWP._dragState && anyWP._dragState.element === el) {
        console.log('[AnyWP] [Drag] END');
        
        const finalRect = el.getBoundingClientRect();
        const finalPos = {
          x: finalRect.left,
          y: finalRect.top
        };
        
        if (persistKey) {
          Storage.saveElementPosition(anyWP, persistKey, finalPos.x, finalPos.y);
        }
        
        if (onDragEnd) {
          onDragEnd(finalPos);
        }
        
        Debug.log('Drag end at: ' + finalPos.x + ',' + finalPos.y, true);
        
        anyWP._dragState = null;
      }
      else if (mouseType === 'mousedown' && !isOver) {
        // Mouse down but not over element
      }
      else if (mouseType === 'mousemove' && anyWP._dragState && anyWP._dragState.element !== el) {
        console.log('[AnyWP] [makeDraggable] mousemove but wrong element');
      }
    }
    
    window.addEventListener('AnyWP:mouse', handleGlobalMouse);
    
    el.__anyWP_dragHandler = {
      handleGlobalMouse: handleGlobalMouse
    };
    
    Debug.log('Element made draggable (via mouse hook): ' + (el.id || el.className));
  },
  
  // Remove draggable functionality
  removeDraggable(anyWP, element) {
    let el = element;
    if (typeof element === 'string') {
      el = document.querySelector(element);
    }
    
    if (!el) {
      console.error('[AnyWP] Element not found:', element);
      return;
    }
    
    if (el.__anyWP_dragHandler) {
      window.removeEventListener('AnyWP:mouse', el.__anyWP_dragHandler.handleGlobalMouse);
      delete el.__anyWP_dragHandler;
    }
    
    anyWP._draggableElements = anyWP._draggableElements.filter(function(d) {
      return d.element !== el;
    });
    
    el.style.cursor = '';
    el.style.userSelect = '';
    
    Debug.log('Removed draggable from element: ' + (el.id || el.className));
  },
  
  // Reset element position
  resetPosition(anyWP, element, position) {
    let el = element;
    if (typeof element === 'string') {
      el = document.querySelector(element);
    }
    
    if (!el) {
      console.error('[AnyWP] Element not found:', element);
      return false;
    }
    
    const draggableData = anyWP._draggableElements.find(function(d) {
      return d.element === el;
    });
    
    if (position) {
      el.style.left = position.left + 'px';
      el.style.top = position.top + 'px';
    } else {
      el.style.left = '';
      el.style.top = '';
    }
    
    if (draggableData && draggableData.persistKey) {
      const key = draggableData.persistKey;
      
      delete anyWP._persistedState[key];
      
      if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
          type: 'saveState',
          key: key,
          value: position ? JSON.stringify(position) : ''
        });
      } else {
        try {
          if (position) {
            localStorage.setItem('AnyWP_' + key, JSON.stringify(position));
          } else {
            localStorage.removeItem('AnyWP_' + key);
          }
        } catch (e) {
          console.warn('[AnyWP] Failed to reset position:', e);
        }
      }
      
      console.log('[AnyWP] Reset position for ' + key);
    }
    
    return true;
  }
};


