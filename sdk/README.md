# AnyWP Engine - Cross-Platform TypeScript SDK

**Version**: 2.2.0  
**Platforms**: Windows, macOS (Linux planned)

## Overview

This is the unified TypeScript SDK for AnyWP Engine, providing a platform-agnostic JavaScript API for web wallpapers. The SDK automatically detects the runtime platform (Windows WebView2 or macOS WKWebView) and adapts accordingly.

## Directory Structure

```
sdk/
├── src/                    # TypeScript source code
│   ├── core/               # Core SDK logic
│   ├── modules/            # Feature modules
│   ├── utils/              # Utility functions
│   │   └── platform.ts     # Platform abstraction layer ⭐
│   ├── types/              # TypeScript type definitions
│   └── index.ts            # Main entry point
├── dist/                   # Compiled JavaScript SDK
│   └── anywp_sdk.js        # Final SDK (used by both platforms)
└── README.md               # This file
```

## Platform Abstraction

The SDK uses a **platform abstraction layer** (`utils/platform.ts`) to handle differences between platforms:

### Windows (WebView2)
- **Message API**: `window.chrome.webview.postMessage()`
- **Event Listener**: `chrome.webview.addEventListener('message')`
- **Runtime**: Chromium-based

### macOS (WKWebView)
- **Message API**: `window.webkit.messageHandlers.anywpMessage.postMessage()`
- **Event Listener**: `window.addEventListener('message')`
- **Runtime**: WebKit-based

### Automatic Detection

```typescript
// Platform is detected automatically
import { detectPlatform, getBridge } from './utils/platform';

const platform = detectPlatform();  // 'windows' | 'macos' | 'unknown'
const bridge = getBridge();          // Platform-specific bridge

// Send message (works on both platforms!)
bridge.postMessage({ type: 'hello', data: {} });
```

## Building the SDK

### Prerequisites

```bash
cd sdk/src
npm install
```

### Build Commands

```bash
# Development build
npm run build:dev

# Production build (minified)
npm run build:production

# Watch mode (auto-rebuild on changes)
npm run build:watch

# Run tests
npm test

# Type checking
npm run typecheck
```

### Build Output

- **src/dist/index.js** - TypeScript compilation output
- **../dist/anywp_sdk.js** - Final SDK bundle (copied automatically)

## Usage in Wallpapers

The SDK is automatically injected by the native plugin, no manual loading required:

```html
<!DOCTYPE html>
<html>
<head>
  <title>My Wallpaper</title>
</head>
<body>
  <h1>Hello AnyWP!</h1>
  
  <script>
    // SDK is available as window.AnyWP
    
    // Platform-independent API
    window.AnyWP.ready('MyWallpaper');
    
    // Send message to Flutter
    window.AnyWP.sendToFlutter('userAction', {
      action: 'click',
      timestamp: Date.now()
    });
    
    // Receive messages from Flutter
    window.AnyWP.onMessage((message) => {
      console.log('Received:', message.type, message.data);
    });
    
    // Check platform
    console.log('Platform:', window.AnyWP.platform);  // 'windows' or 'macos'
    console.log('Version:', window.AnyWP.version);    // '2.2.0'
  </script>
</body>
</html>
```

## API Reference

### Core APIs

```typescript
// Initialization
AnyWP.ready(name: string): void

// Logging
AnyWP.log(message: string): void
AnyWP.enableDebug(): void

// URL Navigation
AnyWP.openURL(url: string): void

// Platform Info
AnyWP.version: string       // SDK version
AnyWP.platform: string      // 'windows' | 'macos'
AnyWP.dpiScale: number      // Device pixel ratio
```

### Bidirectional Communication

```typescript
// Send to Flutter
AnyWP.sendToFlutter(type: string, data?: any): boolean

// Receive from Flutter
AnyWP.onMessage(callback: (message: any) => void): void
```

### State Persistence

```typescript
// Save state
AnyWP.saveState(key: string, value: StateValue): void

// Load state
AnyWP.loadState(key: string, callback: StateLoadCallback): void

// Clear state
AnyWP.clearState(): void
```

### Event Handling

```typescript
// Click events
AnyWP.onClick(element: string | HTMLElement, callback: ClickCallback, options?: Options): void

// Mouse events
AnyWP.onMouse(callback: MouseCallback): void

// Keyboard events
AnyWP.onKeyboard(callback: KeyboardCallback): void

// Visibility changes
AnyWP.onVisibilityChange(callback: (visible: boolean) => void): void
```

### File Operations (v2.1.10+)

```typescript
// Encrypt file
AnyWP.encryptFile(sourcePath: string, destPath: string): Promise<boolean>

// Decrypt file
AnyWP.decryptFile(encryptedPath: string, destPath: string): Promise<boolean>
```

## Architecture Highlights

### 1. Platform-Agnostic Design

All platform-specific code is isolated in `utils/platform.ts`. The rest of the SDK is 100% platform-independent.

### 2. TypeScript First

- Full type safety with TypeScript
- Comprehensive type definitions
- IntelliSense support in IDEs

### 3. Modular Architecture

- Core logic in `core/`
- Feature modules in `modules/`
- Utilities in `utils/`
- Clean separation of concerns

### 4. Automatic Platform Detection

No configuration needed - the SDK automatically detects the runtime platform and uses the appropriate APIs.

### 5. Backward Compatible

Existing Windows wallpapers continue to work without modification.

## Testing

```bash
# Run all tests
npm test

# Watch mode
npm run test:watch

# Coverage report
npm run test:coverage
```

Tests use Jest with jsdom environment for DOM simulation.

## Development Workflow

1. **Edit TypeScript** in `src/`
2. **Build SDK**: `npm run build`
3. **Test in Windows**: Flutter app loads from `windows/` (C++ plugin injects)
4. **Test in macOS**: Flutter app loads from bundle (Objective-C plugin loads)

## Migration Notes

### From v2.1.x to v2.2.0

- ✅ **No breaking changes** - existing code works as-is
- ✅ **New platform detection** - `window.AnyWP.platform` added
- ✅ **macOS support** - same API on both platforms

### SDK Location Change

- **Old**: `windows/sdk/` (TypeScript source)
- **New**: `sdk/src/` (TypeScript source)
- **Compiled**: `sdk/dist/anywp_sdk.js` (final bundle)

## Contributing

When adding new features:

1. Write platform-independent code in `modules/` or `utils/`
2. If platform-specific, update `utils/platform.ts`
3. Add TypeScript types in `types/`
4. Write tests in `__tests__/`
5. Update this README

## License

MIT License - See LICENSE file in project root

---

**Maintained by**: AnyWP Team  
**Last Updated**: 2025-11-17  
**Version**: 2.2.0

