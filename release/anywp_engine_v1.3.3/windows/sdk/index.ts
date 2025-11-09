// AnyWP Engine SDK - Main Entry Point
// Modular architecture with TypeScript support

import { AnyWP } from './core/AnyWP';
import { initializeAnyWP } from './core/init';
import { Debug } from './utils/debug';
import { Events } from './modules/events';
import { ClickHandler } from './modules/click';
import { DragHandler } from './modules/drag';
import { Storage } from './modules/storage';
import { SPA } from './modules/spa';
import type { 
  AnyWPSDK, 
  ClickCallback, 
  ClickHandlerOptions, 
  DraggableOptions,
  StateLoadCallback,
  MouseCallback,
  KeyboardCallback
} from './types';

// Implement initialization
AnyWP._init = function(this: AnyWPSDK) {
  initializeAnyWP(this);
};

// Public API: Debug
AnyWP.enableDebug = function(this: AnyWPSDK) {
  Debug.enable();
};

AnyWP._log = function(this: AnyWPSDK, message: string, always?: boolean) {
  Debug.log(message, always);
};

// Public API: Debug
AnyWP.log = function(this: AnyWPSDK, message: string) {
  Debug.log(message, true);
  
  // Also send to native if available
  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.postMessage({
      type: 'log',
      message: message
    });
  }
};

// Public API: Click Handler
AnyWP.onClick = function(this: AnyWPSDK, element: string | HTMLElement, callback: ClickCallback, options?: ClickHandlerOptions) {
  ClickHandler.onClick(this, element, callback, options);
};

AnyWP.refreshBounds = function(this: AnyWPSDK): number {
  return ClickHandler.refreshBounds(this);
};

AnyWP.clearHandlers = function(this: AnyWPSDK) {
  ClickHandler.clearHandlers(this);
};

// Public API: Drag Handler
AnyWP.makeDraggable = function(this: AnyWPSDK, element: string | HTMLElement, options?: DraggableOptions) {
  DragHandler.makeDraggable(this, element, options);
};

AnyWP.removeDraggable = function(this: AnyWPSDK, element: string | HTMLElement) {
  DragHandler.removeDraggable(this, element);
};

AnyWP.resetPosition = function(this: AnyWPSDK, element: string | HTMLElement, position?: { x: number; y: number }): boolean {
  return DragHandler.resetPosition(this, element, position ? { left: position.x, top: position.y } : undefined);
};

// Public API: Storage
AnyWP.saveState = function(this: AnyWPSDK, key: string, value: any) {
  Storage.save(this, key, value);
};

AnyWP.loadState = function(this: AnyWPSDK, key: string, callback: StateLoadCallback) {
  Storage.load(this, key, callback);
};

AnyWP.clearState = function(this: AnyWPSDK) {
  Storage.clear(this);
};

// Public API: Events
AnyWP.onMouse = function(this: AnyWPSDK, callback: MouseCallback) {
  Events.onMouse(this, callback);
};

AnyWP.onKeyboard = function(this: AnyWPSDK, callback: KeyboardCallback) {
  Events.onKeyboard(this, callback);
};

AnyWP.onVisibilityChange = function(this: AnyWPSDK, callback: (visible: boolean) => void) {
  Events.onVisibilityChange(this, callback);
};

AnyWP._notifyVisibilityChange = function(this: AnyWPSDK, visible: boolean) {
  Events.notifyVisibilityChange(this, visible);
};

// Public API: SPA
AnyWP.setSPAMode = function(this: AnyWPSDK, enabled: boolean) {
  SPA.setSPAMode(this, ClickHandler, enabled);
};

// Public API: Utility
AnyWP.openURL = function(this: AnyWPSDK, url: string) {
  Debug.log('Opening URL: ' + url);
  
  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.postMessage({
      type: 'openURL',
      url: url
    });
  } else {
    console.warn('[AnyWP] Native bridge not available');
    window.open(url, '_blank');
  }
};

AnyWP.ready = function(this: AnyWPSDK, name: string) {
  Debug.log('Wallpaper ready: ' + name, true);
  
  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.postMessage({
      type: 'ready',
      name: name
    });
  }
};

// Export for build
export { AnyWP };
export type { AnyWPSDK };

// Auto-initialize when DOM is ready
if (typeof window !== 'undefined') {
  // ========== CRITICAL: Setup WebMessage listener IMMEDIATELY ==========
  // This must be done BEFORE any other initialization to catch all messages from C++
  if ((window as any).chrome && (window as any).chrome.webview) {
    const globalAny = window as any;
    
    if (globalAny._anywpEarlyMessageListenerRegistered) {
      console.log('[AnyWP] WebMessage listener already registered (EARLY), skipping duplicate');
    } else {
      console.log('[AnyWP] Setting up WebMessage listener (EARLY)');
      globalAny._anywpEarlyMessageListenerRegistered = true;
      
      (window as any).chrome.webview.addEventListener('message', function(event: any) {
        const data = event.data;
        
        // Debug logging for all messages
        if (data && data.type === 'mouseEvent' && data.eventType === 'mousemove') {
          // Log every 100th mousemove to avoid spam
          if (!((window as any)._anywp_msg_count)) {
            (window as any)._anywp_msg_count = 0;
          }
          (window as any)._anywp_msg_count++;
          
          if ((window as any)._anywp_msg_count % 100 === 0) {
            console.log('[AnyWP] WebMessage received #' + (window as any)._anywp_msg_count + ': ' + data.eventType);
          }
        } else if (data && data.type === 'mouseEvent') {
          console.log('[AnyWP] WebMessage: ' + data.eventType + ' at (' + data.x + ',' + data.y + ')');
        }
        
        // Handle mouseEvent messages from C++
        if (data && data.type === 'mouseEvent') {
          // Dispatch as CustomEvent for compatibility with existing code
          const mouseEvent = new CustomEvent('AnyWP:mouse', {
            detail: {
              type: data.eventType,
              x: data.x,
              y: data.y,
              button: data.button || 0
            }
          });
          window.dispatchEvent(mouseEvent);
          
          // Also dispatch click event for mouseup
          if (data.eventType === 'mouseup') {
            const clickEvent = new CustomEvent('AnyWP:click', {
              detail: {
                x: data.x,
                y: data.y
              }
            });
            window.dispatchEvent(clickEvent);
          }
        }
      });
      
      console.log('[AnyWP] WebMessage listener setup complete (EARLY)');
    }
  } else {
    console.log('[AnyWP] chrome.webview not available');
  }
  
  // ========== CRITICAL: Prevent Duplicate SDK Initialization ==========
  // Check if SDK is already loaded (防止重复注入)
  if (typeof (window as any).AnyWP !== 'undefined') {
    console.log('[AnyWP] SDK already loaded, skipping re-initialization');
    console.log('[AnyWP] This is expected when C++ plugin injects SDK multiple times');
  } else {
    console.log('[AnyWP] Initializing SDK for the first time');
    
    window.AnyWP = AnyWP;
    
    if (document.readyState === 'loading') {
      document.addEventListener('DOMContentLoaded', function() {
        AnyWP._init();
      });
    } else {
      AnyWP._init();
    }
    
    console.log('[AnyWP] SDK loaded successfully');
  }
}

