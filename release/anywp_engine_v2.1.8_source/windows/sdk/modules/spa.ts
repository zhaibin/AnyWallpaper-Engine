// SPA framework support module
import { Debug } from '../utils/debug';
import type { AnyWPSDK } from '../types';
import type { ClickHandler as ClickHandlerType } from './click';

declare global {
  interface Window {
    React?: unknown;
    ReactDOM?: unknown;
    Vue?: unknown;
    angular?: unknown;
  }
}

export const SPA = {
  // Detect SPA framework
  detect(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType) {
    const self = this;
    
    // Immediate check for React/Vue/Angular globals
    const isReact = !!(window.React || window.ReactDOM);
    const isVue = !!(window.Vue);
    const isAngular = !!(window.angular);
    
    if (isReact || isVue || isAngular) {
      anyWP._spaMode = true;
      const framework = isReact ? 'React' : (isVue ? 'Vue' : 'Angular');
      console.log('[AnyWP] SPA Framework detected: ' + framework);
      self.setupMonitoring(anyWP, clickHandler);
    } else {
      // Delayed check for DOM elements
      setTimeout(function() {
        const isReactDOM = !!document.querySelector('[data-reactroot], [data-reactid], #root');
        const isVueDOM = !!document.querySelector('[data-v-]');
        const isAngularDOM = !!document.querySelector('[ng-version]');
        
        if (isReactDOM || isVueDOM || isAngularDOM) {
          anyWP._spaMode = true;
          const framework = isReactDOM ? 'React' : (isVueDOM ? 'Vue' : 'Angular');
          console.log('[AnyWP] SPA Framework detected: ' + framework);
          self.setupMonitoring(anyWP, clickHandler);
        }
      }, 500);
    }
  },
  
  // Enable/Disable SPA mode manually
  setSPAMode(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType, enabled: boolean) {
    anyWP._spaMode = enabled;
    console.log('[AnyWP] SPA mode: ' + (enabled ? 'ENABLED' : 'DISABLED'));
    
    if (enabled) {
      this.setupMonitoring(anyWP, clickHandler);
    } else {
      this.teardownMonitoring(anyWP);
    }
  },
  
  // Setup SPA monitoring
  setupMonitoring(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType) {
    const self = this;
    
    // Monitor history changes
    const originalPushState = history.pushState;
    const originalReplaceState = history.replaceState;
    
    history.pushState = function(data: unknown, unused: string, url?: string | URL | null) {
      originalPushState.call(history, data, unused, url);
      self.onRouteChange(anyWP, clickHandler);
    };
    
    history.replaceState = function(data: unknown, unused: string, url?: string | URL | null) {
      originalReplaceState.call(history, data, unused, url);
      self.onRouteChange(anyWP, clickHandler);
    };
    
    window.addEventListener('popstate', function() {
      self.onRouteChange(anyWP, clickHandler);
    });
    
    // Monitor DOM mutations
    if (window.MutationObserver) {
      anyWP._mutationObserver = new MutationObserver(function(mutations: MutationRecord[]) {
        let shouldRefresh = false;
        
        mutations.forEach(function(mutation: MutationRecord) {
          if (mutation.type === 'childList' || mutation.type === 'attributes') {
            anyWP._clickHandlers.forEach(function(handler) {
              if (mutation.target === handler.element || 
                  (mutation.target as Node).contains(handler.element)) {
                shouldRefresh = true;
              }
            });
          }
        });
        
        if (shouldRefresh) {
          clickHandler.refreshBounds(anyWP);
        }
      });
      
      anyWP._mutationObserver.observe(document.body, {
        childList: true,
        subtree: true,
        attributes: true,
        attributeFilter: ['class', 'style']
      });
    }
  },
  
  // Teardown SPA monitoring
  teardownMonitoring(anyWP: AnyWPSDK) {
    if (anyWP._mutationObserver) {
      anyWP._mutationObserver.disconnect();
      anyWP._mutationObserver = null;
    }
  },
  
  // Handle SPA route change
  onRouteChange(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType) {
    Debug.log('Route changed, refreshing...');
    
    setTimeout(function() {
      anyWP._clickHandlers.forEach(function(handler) {
        if (handler.selector && handler.autoRefresh) {
          const newElement = document.querySelector(handler.selector) as HTMLElement | null;
          if (newElement && newElement !== handler.element) {
            handler.element = newElement;
            clickHandler.refreshElementBounds(anyWP, handler);
            
            if (handler.resizeObserver) {
              handler.resizeObserver.disconnect();
            }
            if (window.ResizeObserver) {
              const resizeObserver = new ResizeObserver(function() {
                clickHandler.refreshElementBounds(anyWP, handler);
              });
              resizeObserver.observe(newElement);
              handler.resizeObserver = resizeObserver;
            }
          }
        }
      });
      
      clickHandler.refreshBounds(anyWP);
    }, 500);
  }
};

