// State persistence module
import { Debug } from '../utils/debug';
import { logger } from '../utils/logger';
import type { AnyWPSDK, StateLoadCallback, StateValue } from '../types';

const log = logger.scope('Storage');

export const Storage = {
  /**
   * Load state from native storage
   */
  load(anyWP: AnyWPSDK, key: string, callback: StateLoadCallback): void {
    log.debug('Loading state for key:', key);
    
    // Check local cache first
    if (anyWP._persistedState[key]) {
      log.debug('Found in cache:', anyWP._persistedState[key]);
      callback(anyWP._persistedState[key]);
      return;
    }
    
    // Request from native layer
    if (window.chrome?.webview) {
      log.debug('Requesting state from native layer...');
      
      let resolved = false;
      let timeoutId: number | undefined;
      
      const handler = (event: CustomEvent): void => {
        if (event.detail && event.detail.type === 'stateLoaded' && event.detail.key === key) {
          log.debug('Received stateLoaded event:', event.detail);
          window.removeEventListener('AnyWP:stateLoaded', handler as EventListener);
          
          if (timeoutId !== undefined) {
            clearTimeout(timeoutId);
          }
          resolved = true;
          
          const value = event.detail.value ? JSON.parse(event.detail.value) : null;
          anyWP._persistedState[key] = value;
          log.info('State loaded successfully:', value);
          callback(value);
        }
      };
      
      window.addEventListener('AnyWP:stateLoaded', handler as EventListener);
      
      window.chrome.webview.postMessage({
        type: 'loadState',
        key: key
      });
      
      timeoutId = window.setTimeout(() => {
        if (!resolved) {
          window.removeEventListener('AnyWP:stateLoaded', handler as EventListener);
          log.warn('loadState timeout for key:', key);
          callback(null);
        }
      }, 1000);
    } else {
      log.warn('chrome.webview not available, using localStorage');
      try {
        const stored = localStorage.getItem('AnyWP_' + key);
        const value = stored ? JSON.parse(stored) : null;
        anyWP._persistedState[key] = value;
        log.info('Loaded from localStorage:', value);
        callback(value);
      } catch (e) {
        log.error('Failed to load state:', e);
        callback(null);
      }
    }
  },
  
  /**
   * Save custom state
   */
  save(anyWP: AnyWPSDK, key: string, value: StateValue): void {
    anyWP._persistedState[key] = value;
    
    if (window.chrome?.webview) {
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
  
  /**
   * Clear all saved state
   */
  clear(anyWP: AnyWPSDK): void {
    anyWP._persistedState = {};
    
    if (window.chrome?.webview) {
      window.chrome.webview.postMessage({
        type: 'clearState'
      });
      
      Debug.log('Cleared all saved state');
    } else {
      try {
        const keys = Object.keys(localStorage);
        keys.forEach((key) => {
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
  
  /**
   * Save element position
   */
  saveElementPosition(anyWP: AnyWPSDK, key: string, x: number, y: number): void {
    const position = { left: x, top: y };
    
    log.debug('Saving position for ' + key + ': ', position);
    
    anyWP._persistedState[key] = position;
    
    if (window.chrome?.webview) {
      const msg = {
        type: 'saveState',
        key: key,
        value: JSON.stringify(position)
      };
      log.debug('Sending saveState message:', msg);
      window.chrome.webview.postMessage(msg);
      log.debug('Message sent successfully');
    } else {
      log.warn('chrome.webview not available, using localStorage');
      try {
        localStorage.setItem('AnyWP_' + key, JSON.stringify(position));
        log.info('Saved to localStorage for ' + key);
      } catch (e) {
        log.error('Failed to save state:', e);
      }
    }
  }
};

