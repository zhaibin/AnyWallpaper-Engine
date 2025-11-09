# AnyWP Web SDK v1.3.3

## 📦 Package Contents

This package contains the JavaScript SDK for AnyWP Engine web developers.

```
anywp_web_sdk_v1.3.3/
├── sdk/
│   └── anywp_sdk.js        # JavaScript SDK (built from TypeScript)
├── examples/
│   ├── test_simple.html
│   ├── test_draggable.html
│   ├── test_api.html
│   ├── test_click.html
│   ├── test_visibility.html
│   ├── test_react.html
│   ├── test_vue.html
│   ├── test_iframe_ads.html
│   └── ... (more examples)
├── docs/
│   ├── WEB_DEVELOPER_GUIDE_CN.md    # Chinese guide
│   ├── WEB_DEVELOPER_GUIDE.md       # English guide
│   └── API_USAGE_EXAMPLES.md        # API examples
├── README.md                         # This file
└── LICENSE
```

## 🚀 Quick Start

### Important: SDK Injection

**The SDK is automatically injected by the AnyWP Engine C++ plugin.**

You **DO NOT** need to manually load the SDK in your HTML:

```html
<!-- ❌ DO NOT do this -->
<script src="../windows/anywp_sdk.js"></script>

<!-- ✅ Just use the AnyWP object directly -->
<script>
  if (window.AnyWP) {
    AnyWP.onClick('#myButton', function() {
      console.log('Button clicked!');
    });
  }
</script>
```

### Basic Example

```html
<!DOCTYPE html>
<html>
<head>
  <title>AnyWP Example</title>
  <style>
    #draggable {
      width: 200px;
      height: 100px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border-radius: 10px;
      display: flex;
      align-items: center;
      justify-content: center;
      color: white;
      font-family: Arial, sans-serif;
      cursor: move;
    }
  </style>
</head>
<body>
  <div id="draggable">Drag Me!</div>
  
  <script>
    if (window.AnyWP) {
      // Make element draggable
      AnyWP.makeDraggable('#draggable', {
        saveKey: 'myDraggable',
        onDragStart: function() {
          console.log('Drag started');
        },
        onDragEnd: function() {
          console.log('Drag ended');
        }
      });
    }
  </script>
</body>
</html>
```

## 📚 API Reference

### Core Functions

#### `AnyWP.makeDraggable(selector, options)`
Make an element draggable with position persistence.

**Parameters:**
- `selector` (string): CSS selector for the element
- `options` (object): Configuration options
  - `saveKey` (string): Key for saving position
  - `onDragStart` (function): Callback when drag starts
  - `onDragEnd` (function): Callback when drag ends
  - `onDrag` (function): Callback during dragging

#### `AnyWP.onClick(selector, callback, options)`
Register click handler for an element.

**Parameters:**
- `selector` (string|HTMLElement): CSS selector or element
- `callback` (function): Click handler function
- `options` (object): Configuration options
  - `immediate` (boolean): Execute callback immediately

#### `AnyWP.saveState(key, value)`
Save state data.

**Parameters:**
- `key` (string): State key
- `value` (any): State value (will be JSON.stringify)

#### `AnyWP.loadState(key, callback)`
Load state data.

**Parameters:**
- `key` (string): State key
- `callback` (function): Callback with loaded data

#### `AnyWP.onKeyboard(callback)`
Register keyboard event handler.

**Parameters:**
- `callback` (function): Handler function with event details

#### `AnyWP.onVisibilityChange(callback)`
Register visibility change handler.

**Parameters:**
- `callback` (function): Handler function (true = visible, false = hidden)

## 🔧 v1.3.3 Changes

### SDK Injection Fix
- **Added duplicate injection prevention**: Global flag to prevent WebMessage listener from being registered multiple times
- **Removed manual SDK loading**: All example HTML pages no longer manually load the SDK
- **Consistent event handling**: Fixed issue where click events were firing twice

### Window Detection Improvements
- **Restored full window detection**: Mouse events now properly detect occluding windows
- **Added `IsOurWindow` helper**: Correctly identifies wallpaper windows to prevent false occlusion detection
- **Fixed mouseover after click**: Mouse events continue working properly after clicking within the WebView

### Architecture
- **100% TypeScript SDK**: Built from modular TypeScript source code
- **Single bundle output**: `anywp_sdk.js` is the built artifact from TypeScript
- **118 unit tests**: 96.6% pass rate with comprehensive test coverage

## 📖 Documentation

- **WEB_DEVELOPER_GUIDE_CN.md**: Complete Chinese developer guide
- **WEB_DEVELOPER_GUIDE.md**: Complete English developer guide
- **API_USAGE_EXAMPLES.md**: Practical API usage examples

## 🧪 Testing

Explore the `examples/` directory for various test cases:
- **test_simple.html**: Basic SDK functionality
- **test_draggable.html**: Drag and drop demo
- **test_api.html**: Complete API showcase
- **test_click.html**: Click event testing
- **test_visibility.html**: Power saving demo
- **test_react.html**: React integration
- **test_vue.html**: Vue integration
- **test_iframe_ads.html**: Iframe handling

## 📄 License

See LICENSE file for details.

## 🔗 Links

- **GitHub**: https://github.com/zhaibin/AnyWallpaper-Engine
- **Full Plugin Package**: Download `anywp_engine_v1.3.3.zip` for Flutter integration

---

**Note**: This package is for **web developers only**. If you're developing a Flutter application, download the full `anywp_engine_v1.3.3.zip` package instead.

