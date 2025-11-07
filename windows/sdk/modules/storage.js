// State persistence module
import { Debug } from '../utils/debug.js';

export const Storage = {
  // Load state from native storage
  load(anyWP, key, callback) {
    console.log('[AnyWP] Loading state for key:', key);
    
    // Check local cache first
    if (anyWP._persistedState[key]) {
      console.log('[AnyWP] Found in cache:', anyWP._persistedState[key]);
      callback(anyWP._persistedState[key]);
      return;
    }
    
    // Request from native layer
    if (window.chrome && window.chrome.webview) {
      console.log('[AnyWP] Requesting state from native layer...');
      
      const handler = function(event) {
        if (event.detail && event.detail.type === 'stateLoaded' && event.detail.key === key) {
          console.log('[AnyWP] Received stateLoaded event:', event.detail);
          window.removeEventListener('AnyWP:stateLoaded', handler);
          
          const value = event.detail.value ? JSON.parse(event.detail.value) : null;
          anyWP._persistedState[key] = value;
          console.log('[AnyWP] State loaded successfully:', value);
          callback(value);
        }
      };
      
      window.addEventListener('AnyWP:stateLoaded', handler);
      
      window.chrome.webview.postMessage({
        type: 'loadState',
        key: key
      });
      
      setTimeout(function() {
        window.removeEventListener('AnyWP:stateLoaded', handler);
        console.log('[AnyWP] loadState timeout for key:', key);
        callback(null);
      }, 1000);
    } else {
      console.warn('[AnyWP] chrome.webview not available, using localStorage');
      try {
        const stored = localStorage.getItem('AnyWP_' + key);
        const value = stored ? JSON.parse(stored) : null;
        anyWP._persistedState[key] = value;
        console.log('[AnyWP] Loaded from localStorage:', value);
        callback(value);
      } catch (e) {
        console.error('[AnyWP] Failed to load state:', e);
        callback(null);
      }
    }
  },
  
  // Save custom state
  save(anyWP, key, value) {
    anyWP._persistedState[key] = value;
    
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage({
        type: 'saveState',
        key: key,
        value: JSON.stringify(value)
      });
      
      Debug.log('Saved state for ' + key);
    } else {
      try {
        localStorage.setItem('AnyWP_' + key, JSON.stringify(value));
        Debug.log('Saved state to localStorage for ' + key);
      } catch (e) {
        console.warn('[AnyWP] Failed to save state:', e);
      }
    }
  },
  
  // Clear all saved state
  clear(anyWP) {
    anyWP._persistedState = {};
    
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage({
        type: 'clearState'
      });
      
      Debug.log('Cleared all saved state');
    } else {
      try {
        const keys = Object.keys(localStorage);
        keys.forEach(function(key) {
          if (key.startsWith('AnyWP_')) {
            localStorage.removeItem(key);
          }
        });
        Debug.log('Cleared localStorage state');
      } catch (e) {
        console.warn('[AnyWP] Failed to clear state:', e);
      }
    }
  },
  
  // Save element position
  saveElementPosition(anyWP, key, x, y) {
    const position = { left: x, top: y };
    
    console.log('[AnyWP] Saving position for ' + key + ': ', position);
    
    anyWP._persistedState[key] = position;
    
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
      try {
        localStorage.setItem('AnyWP_' + key, JSON.stringify(position));
        console.log('[AnyWP] Saved to localStorage for ' + key);
      } catch (e) {
        console.error('[AnyWP] Failed to save state:', e);
      }
    }
  }
};

