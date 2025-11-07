# AnyWP Web SDK v1.3.0

**JavaScript SDK for AnyWP Engine - Interactive Desktop Wallpaper Development**

## 📦 Package Contents

```
anywp_web_sdk_v1.3.0/
├── sdk/
│   └── anywp_sdk.js          # Core JavaScript SDK
├── examples/
│   ├── test_simple.html      # Basic wallpaper
│   ├── test_draggable.html   # Draggable elements demo
│   ├── test_api.html         # Complete API showcase
│   ├── test_basic_click.html # Click detection
│   ├── test_visibility.html  # Power-saving visibility API
│   ├── test_react.html       # React SPA example
│   ├── test_vue.html         # Vue SPA example
│   └── test_iframe_ads.html  # iFrame ad detection
├── docs/
│   ├── WEB_DEVELOPER_GUIDE_CN.md  # Chinese guide
│   ├── WEB_DEVELOPER_GUIDE.md     # English guide
│   └── API_USAGE_EXAMPLES.md      # API examples
└── README.md
```

## 🚀 Quick Start

### 1. Include SDK in Your HTML

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>My Wallpaper</title>
</head>
<body>
    <!-- Your content here -->
    
    <!-- Include AnyWP SDK -->
    <script src="anywp_sdk.js"></script>
    <script>
        // SDK is available as window.AnyWP
        if (window.AnyWP) {
            console.log('AnyWP SDK loaded successfully!');
        }
    </script>
</body>
</html>
```

### 2. Basic Usage Examples

#### Make Elements Draggable

```javascript
// Make any element draggable on desktop
AnyWP.makeDraggable('#myElement', {
    onDragEnd: (x, y) => {
        console.log('Element moved to:', x, y);
        AnyWP.saveState('myElement', { x, y });
    }
});
```

#### Handle Click Events

```javascript
// Detect when wallpaper is clicked
AnyWP.onClick((x, y) => {
    console.log('Wallpaper clicked at:', x, y);
});
```

#### Save & Load State

```javascript
// Save data persistently
AnyWP.saveState('myData', { count: 10 });

// Load saved data
const data = AnyWP.loadState('myData');
console.log(data.count); // 10
```

#### Respond to Visibility Changes (Power Saving)

```javascript
// Pause animations when wallpaper is hidden
AnyWP.onVisibilityChange((isVisible) => {
    if (isVisible) {
        console.log('Wallpaper visible - resume animations');
    } else {
        console.log('Wallpaper hidden - pause animations');
    }
});
```

## 📚 Core API Reference

### Dragging
- `AnyWP.makeDraggable(selector, options)` - Make elements draggable
- `AnyWP.setDraggableElements(elements)` - Set multiple draggable elements

### Click Detection
- `AnyWP.onClick(callback)` - Handle wallpaper clicks

### State Management
- `AnyWP.saveState(key, value)` - Save data persistently
- `AnyWP.loadState(key)` - Load saved data
- `AnyWP.clearState(key)` - Clear specific data
- `AnyWP.clearAllState()` - Clear all saved data

### Storage Isolation (v1.2.0+)
- Application-level storage isolation
- Each app has independent storage directory
- Located at: `%LOCALAPPDATA%\AnyWPEngine\[AppName]\`

### Visibility API
- `AnyWP.onVisibilityChange(callback)` - React to visibility changes
- `AnyWP.isVisible()` - Check current visibility

## 🎨 Examples

### 1. Simple Interactive Wallpaper

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <style>
        body {
            margin: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .card {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            cursor: move;
        }
    </style>
</head>
<body>
    <div class="card" id="myCard">
        <h1>Drag Me!</h1>
        <p>This card is draggable on your desktop.</p>
    </div>
    
    <script src="anywp_sdk.js"></script>
    <script>
        // Make card draggable
        AnyWP.makeDraggable('#myCard', {
            onDragEnd: (x, y) => {
                // Save position
                AnyWP.saveState('cardPosition', { x, y });
            }
        });
        
        // Restore saved position on load
        const saved = AnyWP.loadState('cardPosition');
        if (saved) {
            const card = document.getElementById('myCard');
            card.style.position = 'absolute';
            card.style.left = saved.x + 'px';
            card.style.top = saved.y + 'px';
        }
    </script>
</body>
</html>
```

### 2. Power-Saving Animation

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <style>
        .particle {
            width: 10px;
            height: 10px;
            background: white;
            border-radius: 50%;
            position: absolute;
        }
    </style>
</head>
<body>
    <script src="anywp_sdk.js"></script>
    <script>
        let animationId = null;
        
        function animate() {
            // Your animation code here
            animationId = requestAnimationFrame(animate);
        }
        
        // Start/stop animation based on visibility
        AnyWP.onVisibilityChange((isVisible) => {
            if (isVisible) {
                console.log('Wallpaper visible - starting animation');
                animate();
            } else {
                console.log('Wallpaper hidden - stopping animation');
                if (animationId) {
                    cancelAnimationFrame(animationId);
                    animationId = null;
                }
            }
        });
        
        // Start animation initially
        if (AnyWP.isVisible()) {
            animate();
        }
    </script>
</body>
</html>
```

## 🔧 Integration with AnyWP Engine

This SDK is designed to work with **AnyWP Engine** - a Flutter Windows plugin that embeds WebView2 as interactive desktop wallpaper.

### For Flutter Developers

If you're building a wallpaper application using Flutter, you need to integrate the AnyWP Engine plugin:

1. Download the Flutter plugin from: https://github.com/zhaibin/AnyWallpaper-Engine/releases
2. Follow the integration guide in the plugin package
3. Use this Web SDK in your HTML wallpaper files

### Standalone Usage

This SDK can also be used standalone for testing your wallpaper designs before integration:

```bash
# Open any example in your browser
start examples/test_simple.html
```

## 📖 Documentation

- **Web Developer Guide (CN)**: `docs/WEB_DEVELOPER_GUIDE_CN.md`
- **Web Developer Guide (EN)**: `docs/WEB_DEVELOPER_GUIDE.md`
- **API Examples**: `docs/API_USAGE_EXAMPLES.md`

## 🌟 Features

- ✅ **Drag & Drop** - Make any element draggable on desktop
- ✅ **Click Detection** - Handle click events on wallpaper
- ✅ **State Persistence** - Save/load data across sessions
- ✅ **Storage Isolation** - App-level isolated storage (v1.2.0+)
- ✅ **Visibility API** - Power-saving visibility detection
- ✅ **Zero Dependencies** - Pure JavaScript, no external libraries
- ✅ **Framework Agnostic** - Works with React, Vue, plain HTML

## 🔗 Links

- **GitHub**: https://github.com/zhaibin/AnyWallpaper-Engine
- **Flutter Plugin**: Download from Releases page
- **Report Issues**: https://github.com/zhaibin/AnyWallpaper-Engine/issues

## 📄 License

MIT License - See LICENSE file for details

---

**Version**: 1.3.0  
**Release Date**: 2025-11-07  
**Compatibility**: AnyWP Engine v1.0.0+
