// Wallpaper module tests
import { describe, it, expect, beforeEach, jest } from '@jest/globals';
import { initWallpaperController } from '../modules/wallpaper';
import type { AnyWPSDK } from '../types';

describe('Wallpaper module', () => {
  let mockSDK: AnyWPSDK;
  let mockElement: HTMLElement;
  
  beforeEach(() => {
    // Reset DOM
    document.body.innerHTML = '<div id="test-element"></div>';
    mockElement = document.getElementById('test-element')!;
    
    // Mock AnyWP SDK
    mockSDK = {
        version: '2.1.1',
      dpiScale: 1,
      screenWidth: 1920,
      screenHeight: 1080,
      _debugMode: false,
      _clickHandlers: [],
      _mouseCallbacks: [],
      _keyboardCallbacks: [],
      _visibilityCallback: null,
      _mutationObserver: null,
      _resizeObserver: null,
      _spaMode: false,
      _autoRefreshEnabled: true,
      _persistedState: {},
      _onFlutterMessage: null,
      _init: jest.fn(),
      _log: jest.fn(),
      log: jest.fn(),
      enableDebug: jest.fn(),
      onClick: jest.fn(),
      refreshBounds: jest.fn(() => 0),
      clearHandlers: jest.fn(),
      saveState: jest.fn(),
      loadState: jest.fn(),
      clearState: jest.fn(),
      onMouse: jest.fn(),
      onKeyboard: jest.fn(),
      onVisibilityChange: jest.fn(),
      _notifyVisibilityChange: jest.fn(),
      setSPAMode: jest.fn(),
      openURL: jest.fn(),
      ready: jest.fn(),
      sendToFlutter: jest.fn(() => true),
      onMessage: jest.fn()
    };
    
    // Mock chrome.webview
    (window as any).chrome = {
      webview: {
        postMessage: jest.fn()
      }
    };
    
    // Mock window.AnyWP
    (window as any).AnyWP = mockSDK;
  });
  
  describe('initWallpaperController', () => {
    it('should initialize wallpaper controller', () => {
      initWallpaperController(mockSDK);
      
      expect(mockSDK._log).toHaveBeenCalledWith(
        '[Wallpaper] Wallpaper controller initialized successfully',
        true
      );
    });
    
    it('should expose Wallpaper API', () => {
      initWallpaperController(mockSDK);
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      expect(wallpaperAPI).toBeDefined();
      expect(wallpaperAPI.setInteractive).toBeDefined();
      expect(wallpaperAPI.forceMouseOverEvent).toBeDefined();
      expect(wallpaperAPI.getState).toBeDefined();
    });
    
    it('should return correct initial state', () => {
      initWallpaperController(mockSDK);
      
      const state = (window as any).AnyWP.Wallpaper.getState();
      expect(state.isInteractive).toBe(false);
      expect(state.lastMouseX).toBe(0);
      expect(state.lastMouseY).toBe(0);
    });
    
    it('should handle initialization errors', () => {
      // Force error by nullifying document
      const originalAddEventListener = document.addEventListener;
      document.addEventListener = jest.fn(() => {
        throw new Error('Test error');
      });
      
      initWallpaperController(mockSDK);
      
      expect(mockSDK._log).toHaveBeenCalledWith(
        expect.stringContaining('[Wallpaper] ERROR initializing controller'),
        true
      );
      
      // Restore
      document.addEventListener = originalAddEventListener;
    });
  });
  
  describe('setInteractive', () => {
    it('should send setInteractive message to C++', () => {
      initWallpaperController(mockSDK);
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.setInteractive(true);
      
      expect((window as any).chrome.webview.postMessage).toHaveBeenCalledWith({
        type: 'setInteractive',
        interactive: true
      });
    });
    
    it('should update interactive state', () => {
      initWallpaperController(mockSDK);
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.setInteractive(true);
      
      const state = wallpaperAPI.getState();
      expect(state.isInteractive).toBe(true);
    });
    
    it('should handle missing chrome.webview', () => {
      delete (window as any).chrome;
      
      initWallpaperController(mockSDK);
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.setInteractive(true);
      
      expect(mockSDK._log).toHaveBeenCalledWith(
        expect.stringContaining('[Wallpaper] ERROR: chrome.webview not available'),
        true
      );
    });
  });
  
  describe('forceMouseOverEvent', () => {
    it('should dispatch mouseover event at given position', () => {
      initWallpaperController(mockSDK);
      
      const mockDispatchEvent = jest.fn(() => true);
      mockElement.dispatchEvent = mockDispatchEvent as any;
      
      // Mock elementFromPoint
      document.elementFromPoint = jest.fn(() => mockElement);
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.forceMouseOverEvent(100, 200);
      
      expect(document.elementFromPoint).toHaveBeenCalledWith(100, 200);
      expect(mockDispatchEvent).toHaveBeenCalled();
    });
    
    it('should handle null element', () => {
      initWallpaperController(mockSDK);
      
      // Mock elementFromPoint to return null
      document.elementFromPoint = jest.fn(() => null);
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.forceMouseOverEvent(100, 200);
      
      expect(mockSDK._log).toHaveBeenCalledWith(
        expect.stringContaining('[Wallpaper] ERROR: No element at position'),
        true
      );
    });
    
    it('should dispatch custom wallpaper event', () => {
      initWallpaperController(mockSDK);
      
      const mockDocDispatchEvent = jest.fn(() => true);
      document.dispatchEvent = mockDocDispatchEvent as any;
      document.elementFromPoint = jest.fn(() => mockElement);
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.forceMouseOverEvent(100, 200);
      
      // Should dispatch both MouseEvent and CustomEvent
      expect(mockDocDispatchEvent).toHaveBeenCalled();
    });
  });
  
  describe('mouse event handling', () => {
    it('should track mouse position on mousedown', () => {
      initWallpaperController(mockSDK);
      
      const mouseEvent = new MouseEvent('mousedown', {
        clientX: 150,
        clientY: 250,
        bubbles: true
      });
      
      // Dispatch to body element for proper event capture
      document.body.dispatchEvent(mouseEvent);
      
      const state = (window as any).AnyWP.Wallpaper.getState();
      expect(state.lastMouseX).toBe(150);
      expect(state.lastMouseY).toBe(250);
    });
    
    it('should track mouse position on mousemove', () => {
      initWallpaperController(mockSDK);
      
      const mouseEvent = new MouseEvent('mousemove', {
        clientX: 300,
        clientY: 400,
        bubbles: true
      });
      
      // Dispatch to body element for proper event capture
      document.body.dispatchEvent(mouseEvent);
      
      const state = (window as any).AnyWP.Wallpaper.getState();
      expect(state.lastMouseX).toBe(300);
      expect(state.lastMouseY).toBe(400);
    });
    
    it('should handle mouseup and restore transparency', (done) => {
      initWallpaperController(mockSDK);
      
      document.elementFromPoint = jest.fn(() => mockElement);
      
      const mouseEvent = new MouseEvent('mouseup', {
        clientX: 200,
        clientY: 300,
        bubbles: true
      });
      
      document.body.dispatchEvent(mouseEvent);
      
      // restoreTransparency is async, wait a bit
      setTimeout(() => {
        const state = (window as any).AnyWP.Wallpaper.getState();
        expect(state.lastMouseX).toBe(200);
        expect(state.lastMouseY).toBe(300);
        done();
      }, 100);
    }, 10000); // Increase timeout
    
    it('should clear timeout on new mousedown', () => {
      initWallpaperController(mockSDK);
      
      // First mousedown
      const mouseEvent1 = new MouseEvent('mousedown', {
        clientX: 100,
        clientY: 100,
        bubbles: true
      });
      document.body.dispatchEvent(mouseEvent1);
      
      // Second mousedown should clear previous timeout
      const mouseEvent2 = new MouseEvent('mousedown', {
        clientX: 200,
        clientY: 200,
        bubbles: true
      });
      document.body.dispatchEvent(mouseEvent2);
      
      const state = (window as any).AnyWP.Wallpaper.getState();
      expect(state.lastMouseX).toBe(200);
      expect(state.lastMouseY).toBe(200);
    });
  });
  
  describe('error handling', () => {
    it('should handle errors in forceMouseOverEvent', () => {
      initWallpaperController(mockSDK);
      
      // Force error by making elementFromPoint throw
      document.elementFromPoint = jest.fn(() => {
        throw new Error('Test error');
      });
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.forceMouseOverEvent(100, 200);
      
      expect(mockSDK._log).toHaveBeenCalledWith(
        expect.stringContaining('[Wallpaper] EXCEPTION in forceMouseOverEvent'),
        true
      );
    });
    
    it('should handle errors in setInteractive', () => {
      initWallpaperController(mockSDK);
      
      // Force error by making postMessage throw
      (window as any).chrome.webview.postMessage = jest.fn(() => {
        throw new Error('Test error');
      });
      
      const wallpaperAPI = (window as any).AnyWP.Wallpaper;
      wallpaperAPI.setInteractive(true);
      
      expect(mockSDK._log).toHaveBeenCalledWith(
        expect.stringContaining('[Wallpaper] EXCEPTION in setInteractive'),
        true
      );
    });
    
    it('should handle errors in restoreTransparency', (done) => {
      initWallpaperController(mockSDK);
      
      // Force error by making elementFromPoint throw during mouseup
      document.elementFromPoint = jest.fn(() => {
        throw new Error('Test error');
      });
      
      const mouseEvent = new MouseEvent('mouseup', {
        clientX: 200,
        clientY: 300,
        bubbles: true
      });
      
      document.body.dispatchEvent(mouseEvent);
      
      setTimeout(() => {
        expect(mockSDK._log).toHaveBeenCalledWith(
          expect.stringContaining('[Wallpaper] EXCEPTION in forceMouseOverEvent'),
          true
        );
        done();
      }, 100);
    }, 10000); // Increase timeout
  });
});

