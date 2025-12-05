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
 * Detect current platform
 */
export declare function detectPlatform(): Platform;
/**
 * Get platform-specific bridge
 */
export declare function getPlatformBridge(): IPlatformBridge;
/**
 * Get or create platform bridge instance
 */
export declare function getBridge(): IPlatformBridge;
/**
 * Check if running in AnyWP Engine
 */
export declare function isAnyWP(): boolean;
//# sourceMappingURL=platform.d.ts.map