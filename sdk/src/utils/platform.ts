/**
 * Platform Detection and Message Bridge Abstraction
 * 
 * Provides unified interface for communication with native layer
 * across different platforms (Windows WebView2, macOS WKWebView)
 */

export type Platform = 'windows' | 'macos' | 'unknown';

/**
 * Message handler type for receiving messages from native
 */
export type MessageHandler = (data: any) => void;

/**
 * Platform Bridge Interface
 */
export interface IPlatformBridge {
  platform: Platform;
  isAvailable(): boolean;
  postMessage(message: any): void;
  addEventListener(handler: MessageHandler): void;
  removeEventListener(handler: MessageHandler): void;
}

/**
 * Windows WebView2 Bridge
 */
class WindowsBridge implements IPlatformBridge {
  platform: Platform = 'windows';
  private handlers: Set<MessageHandler> = new Set();

  isAvailable(): boolean {
    return !!(window as any).chrome?.webview;
  }

  postMessage(message: any): void {
    if (!this.isAvailable()) {
      throw new Error('Windows WebView2 bridge not available');
    }
    (window as any).chrome.webview.postMessage(message);
  }

  addEventListener(handler: MessageHandler): void {
    if (!this.isAvailable()) {
      throw new Error('Windows WebView2 bridge not available');
    }
    
    const wrappedHandler = (event: any) => {
      handler(event.data);
    };
    
    this.handlers.add(handler);
    (window as any).chrome.webview.addEventListener('message', wrappedHandler);
  }

  removeEventListener(handler: MessageHandler): void {
    // Note: In Windows, we need to keep reference to wrapped handler
    // This is a simplified version
    this.handlers.delete(handler);
  }
}

/**
 * macOS WKWebView Bridge
 */
class MacOSBridge implements IPlatformBridge {
  platform: Platform = 'macos';
  private handlers: Set<MessageHandler> = new Set();

  isAvailable(): boolean {
    return !!(window as any).webkit?.messageHandlers?.anywpMessage;
  }

  postMessage(message: any): void {
    if (!this.isAvailable()) {
      throw new Error('macOS WKWebView bridge not available');
    }
    
    // WKWebView expects string message
    const messageStr = typeof message === 'string' ? message : JSON.stringify(message);
    (window as any).webkit.messageHandlers.anywpMessage.postMessage(messageStr);
  }

  addEventListener(handler: MessageHandler): void {
    if (!this.isAvailable()) {
      throw new Error('macOS WKWebView bridge not available');
    }
    
    // For macOS, we use window.postMessage for native -> JS communication
    const wrappedHandler = (event: MessageEvent) => {
      // Filter out messages not from our native bridge
      if (event.source !== window) {
        return;
      }
      handler(event.data);
    };
    
    this.handlers.add(handler);
    window.addEventListener('message', wrappedHandler);
  }

  removeEventListener(handler: MessageHandler): void {
    this.handlers.delete(handler);
    // Note: In production, we'd need to keep reference to wrapped handler
  }
}

/**
 * Detect current platform
 */
export function detectPlatform(): Platform {
  // Check Windows WebView2
  if ((window as any).chrome?.webview) {
    return 'windows';
  }
  
  // Check macOS WKWebView
  if ((window as any).webkit?.messageHandlers?.anywpMessage) {
    return 'macos';
  }
  
  return 'unknown';
}

/**
 * Get platform-specific bridge
 */
export function getPlatformBridge(): IPlatformBridge {
  const platform = detectPlatform();
  
  switch (platform) {
    case 'windows':
      return new WindowsBridge();
    case 'macos':
      return new MacOSBridge();
    default:
      throw new Error(`Unsupported platform: ${platform}`);
  }
}

/**
 * Global platform bridge instance (singleton)
 */
let bridgeInstance: IPlatformBridge | null = null;

/**
 * Get or create platform bridge instance
 */
export function getBridge(): IPlatformBridge {
  if (!bridgeInstance) {
    bridgeInstance = getPlatformBridge();
  }
  return bridgeInstance;
}

/**
 * Check if running in AnyWP Engine
 */
export function isAnyWP(): boolean {
  const platform = detectPlatform();
  return platform !== 'unknown';
}

