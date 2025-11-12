/**
 * Core module tests (AnyWP object and initialization)
 */
import { describe, test, expect, beforeEach, afterEach, jest } from '@jest/globals';
import { AnyWP } from '../core/AnyWP';
import { initializeAnyWP } from '../core/init';
import type { AnyWPSDK } from '../types';

describe('Core - AnyWP object', () => {
  test('should have correct version', () => {
    expect(AnyWP.version).toBe('2.1.0');
  });
  
  test('should have DPI scale', () => {
    expect(AnyWP.dpiScale).toBeGreaterThan(0);
  });
  
  test('should have screen dimensions', () => {
    // In test environment (jsdom), screen dimensions might be 0
    expect(AnyWP.screenWidth).toBeGreaterThanOrEqual(0);
    expect(AnyWP.screenHeight).toBeGreaterThanOrEqual(0);
  });
  
  test('should initialize with empty handlers', () => {
    expect(AnyWP._clickHandlers).toBeDefined();
    expect(AnyWP._mouseCallbacks).toBeDefined();
    expect(AnyWP._keyboardCallbacks).toBeDefined();
  });
  
  test('should have debug mode disabled by default', () => {
    expect(AnyWP._debugMode).toBe(false);
  });
  
  test('should have empty persisted state by default', () => {
    expect(AnyWP._persistedState).toBeDefined();
    expect(typeof AnyWP._persistedState).toBe('object');
  });
  
  test('should have SPA mode disabled by default', () => {
    expect(AnyWP._spaMode).toBe(false);
  });
  
  test('should have auto refresh enabled by default', () => {
    expect(AnyWP._autoRefreshEnabled).toBe(true);
  });
});

describe('Core - Initialization', () => {
  let mockAnyWP: AnyWPSDK;
  let consoleSpy: jest.SpiedFunction<typeof console.log>;
  
  beforeEach(() => {
    // Create a mock AnyWP object
    mockAnyWP = {
      version: '2.1.0',
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
      _log: jest.fn()
    } as any;
    
    // Spy on console.log
    consoleSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
  });
  
  afterEach(() => {
    consoleSpy.mockRestore();
  });
  
  test('should log initialization message', () => {
    initializeAnyWP(mockAnyWP);
    
    // Logger now outputs with prefix [AnyWP] [INFO]
    expect(consoleSpy).toHaveBeenCalledWith(
      expect.stringContaining('[AnyWP]'),
      expect.stringContaining('[INFO]'),
      expect.stringContaining('AnyWP Engine v2.0.0')
    );
  });
  
  test('should log screen dimensions', () => {
    initializeAnyWP(mockAnyWP);
    
    expect(consoleSpy).toHaveBeenCalledWith(
      expect.stringContaining('[AnyWP]'),
      expect.stringContaining('[INFO]'),
      expect.stringContaining('Screen: 1920x1080')
    );
  });
  
  test('should log DPI scale', () => {
    initializeAnyWP(mockAnyWP);
    
    expect(consoleSpy).toHaveBeenCalledWith(
      expect.stringContaining('[AnyWP]'),
      expect.stringContaining('[INFO]'),
      expect.stringContaining('DPI Scale: 1x')
    );
  });
  
  test('should enable debug mode automatically', () => {
    initializeAnyWP(mockAnyWP);
    
    expect(mockAnyWP._debugMode).toBe(true);
  });
  
  test('should log debug mode enabled message', () => {
    initializeAnyWP(mockAnyWP);
    
    expect(consoleSpy).toHaveBeenCalledWith(
      expect.stringContaining('[AnyWP]'),
      expect.stringContaining('[INFO]'),
      expect.stringContaining('Debug mode ENABLED automatically')
    );
  });
});

describe('Core - Error handling', () => {
  test('_init should throw error by default', () => {
    expect(() => {
      AnyWP._init();
    }).toThrow('_init must be implemented');
  });
  
  test('_log should throw error by default', () => {
    expect(() => {
      AnyWP._log('test');
    }).toThrow('Not implemented');
  });
  
  test('log should throw error by default', () => {
    expect(() => {
      AnyWP.log('test');
    }).toThrow('Not implemented');
  });
  
  test('enableDebug should throw error by default', () => {
    expect(() => {
      AnyWP.enableDebug();
    }).toThrow('Not implemented');
  });
  
  test('onClick should throw error by default', () => {
    expect(() => {
      AnyWP.onClick('test-element', () => {});
    }).toThrow('Not implemented');
  });
  
  test('refreshBounds should throw error by default', () => {
    expect(() => {
      AnyWP.refreshBounds();
    }).toThrow('Not implemented');
  });
  
  test('clearHandlers should throw error by default', () => {
    expect(() => {
      AnyWP.clearHandlers();
    }).toThrow('Not implemented');
  });
  
  test('saveState should throw error by default', () => {
    expect(() => {
      AnyWP.saveState('key', {});
    }).toThrow('Not implemented');
  });
  
  test('loadState should throw error by default', () => {
    expect(() => {
      AnyWP.loadState('key', () => {});
    }).toThrow('Not implemented');
  });
  
  test('clearState should throw error by default', () => {
    expect(() => {
      AnyWP.clearState();
    }).toThrow('Not implemented');
  });
});

describe('Core - Utility methods', () => {
  let mockWebview: any;
  
  beforeEach(() => {
    // Mock chrome.webview
    mockWebview = {
      postMessage: jest.fn()
    };
    
    (window as any).chrome = {
      webview: mockWebview
    };
  });
  
  afterEach(() => {
    delete (window as any).chrome;
  });
  
  test('openURL should post message to WebView', () => {
    const consoleSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    
    AnyWP.openURL('https://example.com');
    
    expect(mockWebview.postMessage).toHaveBeenCalledWith({
      type: 'openURL',
      url: 'https://example.com'
    });
    
    consoleSpy.mockRestore();
  });
  
  test('openURL should log message', () => {
    const consoleSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    
    AnyWP.openURL('https://example.com');
    
    expect(consoleSpy).toHaveBeenCalledWith(
      expect.stringContaining('Opening URL: https://example.com')
    );
    
    consoleSpy.mockRestore();
  });
  
  test('ready should post message to WebView', () => {
    const consoleSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    
    AnyWP.ready('TestWallpaper');
    
    expect(mockWebview.postMessage).toHaveBeenCalledWith({
      type: 'ready',
      name: 'TestWallpaper'
    });
    
    consoleSpy.mockRestore();
  });
  
  test('ready should log message', () => {
    const consoleSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    
    AnyWP.ready('TestWallpaper');
    
    expect(consoleSpy).toHaveBeenCalledWith(
      expect.stringContaining('Wallpaper ready: TestWallpaper')
    );
    
    consoleSpy.mockRestore();
  });
  
  test('openURL should fallback to window.open if WebView not available', () => {
    delete (window as any).chrome;
    
    const windowOpenSpy = jest.spyOn(window, 'open').mockImplementation(() => null);
    const consoleWarnSpy = jest.spyOn(console, 'warn').mockImplementation(() => {});
    const consoleLogSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    
    AnyWP.openURL('https://example.com');
    
    expect(consoleWarnSpy).toHaveBeenCalledWith(
      expect.stringContaining('Native bridge not available')
    );
    expect(windowOpenSpy).toHaveBeenCalledWith('https://example.com', '_blank');
    
    windowOpenSpy.mockRestore();
    consoleWarnSpy.mockRestore();
    consoleLogSpy.mockRestore();
  });
});

