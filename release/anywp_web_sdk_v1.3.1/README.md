# AnyWP Web SDK v1.3.1

JavaScript SDK for creating interactive desktop wallpapers with AnyWP Engine.

## 馃摝 What's Included

- **SDK**: sdk/anywp_sdk.js - Core JavaScript SDK
- **Examples**: 8 sample HTML pages demonstrating SDK features
- **Documentation**: Complete guides in Chinese and English

## 馃殌 Quick Start

### 1. Include the SDK

``html
<!DOCTYPE html>
<html>
<head>
    <script src="anywp_sdk.js"></script>
</head>
<body>
    <!-- Your content here -->
</body>
</html>
``

### 2. Use SDK Features

``javascript
// Make elements draggable
window.AnyWP.makeDraggable('my-element');

// Save state to Windows Registry
window.AnyWP.saveState('my-key', { x: 100, y: 200 });

// Load saved state
const state = window.AnyWP.loadState('my-key');

// Handle clicks
window.AnyWP.onClick('my-button', () => {
    console.log('Button clicked!');
});

// Detect visibility changes
window.AnyWP.onVisibilityChange((isVisible) => {
    console.log('Wallpaper visible:', isVisible);
});
``

## 馃摎 Documentation

- **Chinese Guide**: docs/WEB_DEVELOPER_GUIDE_CN.md
- **English Guide**: docs/WEB_DEVELOPER_GUIDE.md
- **API Examples**: docs/API_USAGE_EXAMPLES.md

## 馃帹 Example Files

1. **test_simple.html** - Basic wallpaper
2. **test_draggable.html** - Draggable elements demo
3. **test_api.html** - Complete API showcase
4. **test_click.html** - Click event handling
5. **test_visibility.html** - Power saving demo
6. **test_react.html** - React integration
7. **test_vue.html** - Vue.js integration
8. **test_iframe_ads.html** - iFrame handling

## 馃敡 Core API

### Element Dragging
``javascript
window.AnyWP.makeDraggable(elementId, options)
``

### State Persistence
``javascript
window.AnyWP.saveState(key, data)
window.AnyWP.loadState(key)
window.AnyWP.clearState(key)
``

### Event Handling
``javascript
window.AnyWP.onClick(elementId, callback)
window.AnyWP.onVisibilityChange(callback)
``

### Utility
``javascript
window.AnyWP.getStoragePath()
window.AnyWP.log(message)
``

## 鈿狅笍 Requirements

- **AnyWP Engine**: v1.3.0 or higher
- **Operating System**: Windows 10/11
- **WebView2 Runtime**: Automatically included

## 馃摉 Full Documentation

For complete documentation, please refer to:
- [Flutter Developer Guide](https://github.com/zhaibin/AnyWallpaper-Engine)
- [Web Developer Guide](docs/WEB_DEVELOPER_GUIDE_CN.md)

## 馃搫 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 馃敆 Links

- **GitHub**: https://github.com/zhaibin/AnyWallpaper-Engine
- **Releases**: https://github.com/zhaibin/AnyWallpaper-Engine/releases

---

**Version**: 1.3.1 | **Release Date**: 2025-11-07
