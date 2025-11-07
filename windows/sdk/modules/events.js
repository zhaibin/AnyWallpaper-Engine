// Event system module
import { Debug } from '../utils/debug.js';

export const Events = {
  // Setup event listeners
  setup(anyWP, clickHandler, animationsHandler) {
    window.addEventListener('AnyWP:mouse', function(event) {
      anyWP._mouseCallbacks.forEach(function(cb) {
        cb(event.detail);
      });
    });
    
    window.addEventListener('AnyWP:keyboard', function(event) {
      anyWP._keyboardCallbacks.forEach(function(cb) {
        cb(event.detail);
      });
    });
    
    window.addEventListener('AnyWP:click', function(event) {
      clickHandler.handleClick(event.detail.x, event.detail.y);
    });
    
    window.addEventListener('AnyWP:interactionMode', function(event) {
      anyWP.interactionEnabled = event.detail.enabled;
      Debug.log('Interaction mode: ' + (anyWP.interactionEnabled ? 'ON' : 'OFF'), true);
    });
    
    window.addEventListener('AnyWP:visibility', function(event) {
      const visible = event.detail.visible;
      Debug.log('Visibility changed: ' + (visible ? 'visible' : 'hidden'), true);
      
      if (anyWP._visibilityCallback) {
        anyWP._visibilityCallback(visible);
      }
      
      if (!visible) {
        animationsHandler.pause();
      } else {
        animationsHandler.resume();
      }
    });
    
    window.addEventListener('resize', function() {
      Debug.log('Window resized, refreshing...');
      setTimeout(function() {
        clickHandler.refreshBounds();
      }, 200);
    });
  },
  
  // Register mouse callback
  onMouse(anyWP, callback) {
    anyWP._mouseCallbacks.push(callback);
    Debug.log('Mouse callback registered');
  },
  
  // Register keyboard callback
  onKeyboard(anyWP, callback) {
    anyWP._keyboardCallbacks.push(callback);
    Debug.log('Keyboard callback registered');
  },
  
  // Register visibility callback
  onVisibilityChange(anyWP, callback) {
    anyWP._visibilityCallback = callback;
    Debug.log('Visibility callback registered');
  },
  
  // Notify visibility change (called by C++)
  notifyVisibilityChange(anyWP, visible) {
    console.log('[AnyWP] _notifyVisibilityChange called:', visible);
    
    if (anyWP._visibilityCallback && typeof anyWP._visibilityCallback === 'function') {
      try {
        anyWP._visibilityCallback(visible);
        console.log('[AnyWP] Visibility callback executed successfully');
      } catch(e) {
        console.error('[AnyWP] Error in visibility callback:', e);
      }
    } else {
      console.log('[AnyWP] No visibility callback registered');
    }
    
    try {
      var event = new CustomEvent('AnyWP:visibility', {
        detail: { visible: visible }
      });
      window.dispatchEvent(event);
      console.log('[AnyWP] AnyWP:visibility event dispatched');
    } catch(e) {
      console.error('[AnyWP] Error dispatching visibility event:', e);
    }
  }
};

