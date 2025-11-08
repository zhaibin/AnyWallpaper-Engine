// Event system module
import { Debug } from '../utils/debug';
import type { AnyWPSDK, MouseCallback, KeyboardCallback } from '../types';
import type { ClickHandler as ClickHandlerType } from './click';
import type { Animations as AnimationsType } from './animations';

interface AnyWPMouseEventDetail {
  type: 'mousedown' | 'mousemove' | 'mouseup';
  x: number;
  y: number;
  [key: string]: any;
}

interface AnyWPKeyboardEventDetail {
  type: 'keydown' | 'keyup';
  key: string;
  code: string;
  [key: string]: any;
}

interface AnyWPClickEventDetail {
  x: number;
  y: number;
}

interface AnyWPInteractionEventDetail {
  enabled: boolean;
}

interface AnyWPVisibilityEventDetail {
  visible: boolean;
}

export const Events = {
  // Setup event listeners
  setup(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType, animationsHandler: typeof AnimationsType) {
    window.addEventListener('AnyWP:mouse', function(event: Event) {
      const customEvent = event as CustomEvent<AnyWPMouseEventDetail>;
      anyWP._mouseCallbacks.forEach(function(cb: MouseCallback) {
        cb(customEvent.detail);
      });
    });
    
    window.addEventListener('AnyWP:keyboard', function(event: Event) {
      const customEvent = event as CustomEvent<AnyWPKeyboardEventDetail>;
      anyWP._keyboardCallbacks.forEach(function(cb: KeyboardCallback) {
        cb(customEvent.detail);
      });
    });
    
    window.addEventListener('AnyWP:click', function(event: Event) {
      const customEvent = event as CustomEvent<AnyWPClickEventDetail>;
      clickHandler.handleClick(customEvent.detail.x, customEvent.detail.y);
    });
    
    window.addEventListener('AnyWP:interactionMode', function(event: Event) {
      const customEvent = event as CustomEvent<AnyWPInteractionEventDetail>;
      anyWP.interactionEnabled = customEvent.detail.enabled;
      Debug.log('Interaction mode: ' + (anyWP.interactionEnabled ? 'ON' : 'OFF'), true);
    });
    
    window.addEventListener('AnyWP:visibility', function(event: Event) {
      const customEvent = event as CustomEvent<AnyWPVisibilityEventDetail>;
      const visible = customEvent.detail.visible;
      Debug.log('Visibility changed: ' + (visible ? 'visible' : 'hidden'), true);
      
      if (anyWP._visibilityCallback) {
        anyWP._visibilityCallback(visible);
      }
      
      if (!visible) {
        animationsHandler.pause(anyWP);
      } else {
        animationsHandler.resume(anyWP);
      }
    });
    
    window.addEventListener('resize', function() {
      Debug.log('Window resized, refreshing...');
      setTimeout(function() {
        clickHandler.refreshBounds(anyWP);
      }, 200);
    });
  },
  
  // Register mouse callback
  onMouse(anyWP: AnyWPSDK, callback: MouseCallback) {
    anyWP._mouseCallbacks.push(callback);
    Debug.log('Mouse callback registered');
  },
  
  // Register keyboard callback
  onKeyboard(anyWP: AnyWPSDK, callback: KeyboardCallback) {
    anyWP._keyboardCallbacks.push(callback);
    Debug.log('Keyboard callback registered');
  },
  
  // Register visibility callback
  onVisibilityChange(anyWP: AnyWPSDK, callback: (visible: boolean) => void) {
    anyWP._visibilityCallback = callback;
    Debug.log('Visibility callback registered');
  },
  
  // Notify visibility change (called by C++)
  notifyVisibilityChange(anyWP: AnyWPSDK, visible: boolean) {
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
      const event = new CustomEvent('AnyWP:visibility', {
        detail: { visible: visible }
      });
      window.dispatchEvent(event);
      console.log('[AnyWP] AnyWP:visibility event dispatched');
    } catch(e) {
      console.error('[AnyWP] Error dispatching visibility event:', e);
    }
  }
};

