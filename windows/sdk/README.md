# AnyWP Engine SDK - TypeScript Edition

**Version**: 2.1.1  
**Language**: TypeScript  
**Build**: Modular architecture with Rollup  
**Testing**: Jest + ts-jest  
**Test Coverage**: 100% pass rate (197/197 tests)

## Overview

AnyWP Engine SDK is a fully TypeScript-based JavaScript SDK for creating interactive desktop wallpapers using WebView2. It provides a rich set of APIs for click handling, state persistence, animation control, SPA framework support, and now **configurable logging**.

## What's New in v2.0.0 🎉

### Logging System Enhancement
- 📝 **Unified Logging** - All modules use consistent logger interface
- ⚙️ **Configurable Levels** - ERROR, WARN, INFO, DEBUG
- 🔧 **Flexible Config** - URL parameters or localStorage
- 🏷️ **Module-scoped** - Auto module identification in logs
- 🔄 **Auto Adaptation** - DEBUG for dev, INFO for production

### Technical Improvements
- ✅ Eliminated code duplication
- ✅ Proper logger initialization
- ✅ 197/197 tests passed (100%)

## Features

- ✅ **100% TypeScript** - Full type safety with `.d.ts` declarations
- ✅ **Modular Architecture** - Clean separation of concerns
- ✅ **Type Definitions** - Complete IntelliSense support
- ✅ **Unit Tested** - Jest test suite with 100% pass rate (197 tests)
- ✅ **Configurable Logging** - Flexible log levels for dev/prod 🆕
- ✅ **Zero Dependencies** - Pure TypeScript implementation
- ✅ **ESM + IIFE** - Modern module system with browser bundle

## Installation

```bash
npm install
```

## Development

### Build Commands

```bash
# Full build (clean + compile + bundle)
npm run build

# Watch mode
npm run build:watch

# Production build (with minification)
npm run build:production

# TypeScript compilation only
npm run compile

# Type checking
npm run typecheck
```

### Testing

```bash
# Run all tests
npm test

# Watch mode
npm run test:watch

# Coverage report
npm run test:coverage
```

## Project Structure

```
windows/sdk/
├── core/
│   ├── AnyWP.ts            # Core SDK object
│   └── init.ts             # Initialization logic
├── modules/
│   ├── animations.ts       # Animation control
│   ├── click.ts            # Click handling
│   ├── drag.ts             # Drag & drop
│   ├── events.ts           # Event system
│   ├── spa.ts              # SPA support
│   └── storage.ts          # State persistence
├── utils/
│   ├── bounds.ts           # Bounds calculation
│   ├── coordinates.ts      # Coordinate conversion
│   └── debug.ts            # Debug utilities
├── __tests__/              # Jest test suites
│   ├── bounds.test.ts
│   ├── coordinates.test.ts
│   └── debug.test.ts
├── types.ts                # TypeScript definitions
├── index.ts                # Entry point
├── tsconfig.json           # TypeScript config
├── jest.config.js          # Jest config
└── rollup.config.js        # Rollup config
```

## Type Definitions

The SDK generates complete TypeScript declarations (`.d.ts` files) for use in TypeScript projects:

```typescript
import { AnyWP, type AnyWPSDK } from './windows/sdk';

// Full IntelliSense support
AnyWP.onClick('#button', (x, y) => {
  console.log('Clicked at:', x, y);
});
```

## Logging Configuration 🆕

The SDK supports configurable log levels for better debugging experience:

### Configuration Methods

**Method 1: URL Parameter**
```
file:///path/to/wallpaper.html?loglevel=DEBUG
```

**Method 2: localStorage**
```javascript
localStorage.setItem('anywp_loglevel', 'WARN');
// Reload wallpaper to take effect
```

### Log Levels

| Level | Description | Output |
|-------|-------------|--------|
| `ERROR` | Errors only | Minimal logs |
| `WARN` | Warnings + errors | Important issues |
| `INFO` | General info | Default level |
| `DEBUG` | Detailed debug | Full logging |

### Default Behavior

- **Development** (localhost/127.0.0.1): `DEBUG` level
- **Production**: `INFO` level

### Log Format

All logs follow a consistent format:

```
[AnyWP] [LEVEL] [Module] Message
```

Examples:
```
[AnyWP] [INFO] Wallpaper ready: My Wallpaper
[AnyWP] [DEBUG] [Storage] Loading state for key: position
[AnyWP] [WARN] [WebMessage] chrome.webview not available
[AnyWP] [ERROR] [Click] Element not found: #button
```

### Usage in Code

```javascript
// Use AnyWP.log() for application-level logs
AnyWP.log('User action completed');

// Internal SDK logs are automatically formatted
// and respect the configured log level
```

## Testing

The SDK includes a comprehensive test suite covering all major modules:

**Utility Modules**:
- **Debug** (7 tests) - Logging, borders, URL detection
- **Bounds** (40 tests) - DPI scaling, point-in-bounds checks, edge cases
- **Coordinates** (5 tests) - Screen-to-viewport conversion

**Core Modules**:
- **Core** (29 tests) - SDK object, initialization, utility methods
- **Storage** (12 tests) - State persistence, WebView2/localStorage fallback
- **Animations** (15 tests) - Pause/resume, video/audio control
- **Events** (14 tests) - Mouse, keyboard, visibility events
- **Click** (17 tests) - Click handling, bounds refresh, ResizeObserver
- **Click Extra** (19 tests) - Parameter validation, auto-refresh, debug mode
- **Wallpaper** (17 tests) - Wallpaper controller, mouse tracking
- **SPA** (22 tests) - Framework detection, history interception, DOM monitoring

**Test Coverage**:
- ✅ **197/197 tests passing (100%)**
- ✅ **11 test suites**
- ✅ **All major functionality tested**
- ✅ **Comprehensive error handling tests**

## Build Output

```
dist/                      # TypeScript compilation
  ├── index.d.ts          # Main type definitions
  ├── index.js            # Compiled JavaScript
  ├── types.d.ts          # Type definitions
  └── [modules/utils]     # Compiled modules

../anywp_sdk.js           # Bundled SDK (IIFE)
../anywp_sdk.min.js       # Minified (production)
```

## Usage in Projects

### TypeScript Projects

```typescript
import { AnyWP } from './windows/sdk';

// Type-safe API calls
AnyWP.onClick(element, (x: number, y: number) => {
  // TypeScript knows the parameter types
});
```

### JavaScript Projects

```html
<script src="windows/anywp_sdk.js"></script>
<script>
  // Use global AnyWP object
  AnyWP.onClick('#button', function(x, y) {
    console.log('Clicked:', x, y);
  });
</script>
```

## API Documentation

See the [main documentation](../../docs/) for complete API reference:

- **Developer API Reference**: `docs/DEVELOPER_API_REFERENCE.md`
- **Web Developer Guide**: `docs/WEB_DEVELOPER_GUIDE_CN.md`

## Contributing

### Code Style

- Follow TypeScript best practices
- Write unit tests for new features
- Run `npm run typecheck` before committing
- Ensure all tests pass with `npm test`

### Adding New Features

1. Create TypeScript files in appropriate directory (`core/`, `modules/`, `utils/`)
2. Add type definitions to `types.ts`
3. Write unit tests in `__tests__/`
4. Update this README
5. Run full build and test suite

## License

MIT

---

**Built with ❤️ using TypeScript**
