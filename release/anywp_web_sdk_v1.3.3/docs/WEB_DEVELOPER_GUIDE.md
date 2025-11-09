# AnyWP Engine - Web Developer Integration Guide

## 📖 Overview

This guide is for web developers who want to integrate their web pages into AnyWP Engine as interactive desktop wallpapers.

**Supported Tech Stack**:
- ✅ Native HTML/CSS/JavaScript
- ✅ React (including Create React App, Next.js, etc.)
- ✅ Vue 2/3 (including Nuxt.js, etc.)
- ✅ Angular
- ✅ Any other web framework

---

## 🚀 Quick Start

### 1. Basic Integration (Static Web Page)

```html
<!DOCTYPE html>
<html>
<head>
  <title>My Wallpaper</title>
</head>
<body>
  <button id="myButton">Click Me</button>

  <script>
    // SDK is auto-injected, use directly
    if (window.AnyWP) {
      // Notify wallpaper is ready
      AnyWP.ready('My Wallpaper');

      // Register clickable area
      AnyWP.onClick('#myButton', function(x, y) {
        console.log('Button clicked!');
        AnyWP.openURL('https://example.com');
      });
    }
  </script>
</body>
</html>
```

---

## 🎯 Core API

### AnyWP Object

The SDK automatically injects the `window.AnyWP` global object.

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `version` | string | SDK version |
| `dpiScale` | number | DPI scaling ratio |
| `screenWidth` | number | Physical screen width (pixels) |
| `screenHeight` | number | Physical screen height (pixels) |
| `interactionEnabled` | boolean | Whether interaction mode is enabled |

#### Methods

##### `ready(name)`
Notify AnyWP Engine that the wallpaper is ready.

```javascript
AnyWP.ready('My Awesome Wallpaper');
```

##### `onClick(element, callback, options)`
Register a clickable area.

**Parameters**:
- `element`: Element selector (string) or DOM element object
- `callback(x, y)`: Click callback function
- `options`: Configuration options (optional)

**Options**:
```javascript
{
  immediate: false,      // Register immediately (no wait)
  waitFor: true,         // Wait for element to appear (recommended for SPA)
  maxWait: 10000,        // Maximum wait time (milliseconds)
  autoRefresh: true,     // Auto refresh bounds
  delay: 100,            // Registration delay (milliseconds)
  debug: false           // Show debug borders
}
```

**Examples**:
```javascript
// Basic usage
AnyWP.onClick('#button', function(x, y) {
  console.log('Click position:', x, y);
});

// Wait for element (SPA recommended)
AnyWP.onClick('.dynamic-button', callback, { 
  waitFor: true,
  maxWait: 5000 
});

// Immediate + debug mode
AnyWP.onClick(document.querySelector('#btn'), callback, { 
  immediate: true,
  debug: true 
});
```

##### `openURL(url)`
Open URL in default browser.

```javascript
AnyWP.openURL('https://github.com');
```

##### `refreshBounds()`
Refresh click bounds for all registered elements.

```javascript
// Call after SPA route change
AnyWP.refreshBounds();
```

##### `clearHandlers()`
Clear all registered click handlers.

```javascript
AnyWP.clearHandlers();
```

##### `enableDebug()`
Enable debug mode (shows red borders for click areas).

```javascript
AnyWP.enableDebug();
```

##### `setSPAMode(enabled)`
Manually enable/disable SPA mode.

```javascript
AnyWP.setSPAMode(true);  // Enable
```

##### `onMouse(callback)`
Listen to mouse events.

```javascript
AnyWP.onMouse(function(event) {
  console.log('Mouse:', event);
});
```

##### `onKeyboard(callback)`
Listen to keyboard events.

```javascript
AnyWP.onKeyboard(function(event) {
  console.log('Keyboard:', event);
});
```

---

## ⚛️ React Integration

### Using useEffect Hook

```jsx
import React, { useEffect, useState } from 'react';

function MyWallpaper() {
  const [count, setCount] = useState(0);

  useEffect(() => {
    if (!window.AnyWP) return;

    // Initialize
    AnyWP.enableDebug();
    AnyWP.ready('React Wallpaper');

    // Register click areas (use waitFor)
    AnyWP.onClick('.increment-btn', () => {
      setCount(c => c + 1);
    }, { waitFor: true });

    AnyWP.onClick('.open-link', () => {
      AnyWP.openURL('https://react.dev');
    }, { waitFor: true });

    // Cleanup function (on component unmount)
    return () => {
      console.log('Component unmounting');
    };
  }, []); // Empty dependency array - register only once

  return (
    <div>
      <h1>Counter: {count}</h1>
      <button className="increment-btn">Increment</button>
      <button className="open-link">Open React Docs</button>
    </div>
  );
}
```

### React Router Navigation

```jsx
import { useEffect } from 'react';
import { useLocation } from 'react-router-dom';

function App() {
  const location = useLocation();

  useEffect(() => {
    // Refresh bounds on route change
    if (window.AnyWP) {
      setTimeout(() => {
        AnyWP.refreshBounds();
      }, 500);
    }
  }, [location]);

  return <Routes>...</Routes>;
}
```

### Best Practices

1. **Initialize once in top-level component**
   ```jsx
   // App.jsx
   useEffect(() => {
     if (window.AnyWP) {
       AnyWP.ready('My App');
       AnyWP.setSPAMode(true);
     }
   }, []);
   ```

2. **Use `waitFor` option**
   ```jsx
   AnyWP.onClick('.my-button', callback, { waitFor: true });
   ```

3. **Avoid state in dependency array**
   ```jsx
   // ❌ Wrong - re-registers on every count change
   useEffect(() => {
     AnyWP.onClick('#btn', () => setCount(count + 1));
   }, [count]);

   // ✅ Correct - register once, use functional update
   useEffect(() => {
     AnyWP.onClick('#btn', () => setCount(c => c + 1), { waitFor: true });
   }, []);
   ```

---

## 💚 Vue Integration

### Vue 3 Composition API

```vue
<template>
  <div>
    <h1>Counter: {{ count }}</h1>
    <button class="increment-btn">Increment</button>
    <button class="open-link">Open Vue Docs</button>
  </div>
</template>

<script>
import { ref, onMounted, onUnmounted } from 'vue';

export default {
  setup() {
    const count = ref(0);

    onMounted(() => {
      if (!window.AnyWP) return;

      // Initialize
      AnyWP.enableDebug();
      AnyWP.ready('Vue Wallpaper');

      // Register click areas
      AnyWP.onClick('.increment-btn', () => {
        count.value++;
      }, { waitFor: true });

      AnyWP.onClick('.open-link', () => {
        AnyWP.openURL('https://vuejs.org');
      }, { waitFor: true });
    });

    onUnmounted(() => {
      console.log('Component unmounting');
    });

    return { count };
  }
};
</script>
```

### Vue 2 Options API

```vue
<script>
export default {
  data() {
    return {
      count: 0
    };
  },
  
  mounted() {
    if (!window.AnyWP) return;

    AnyWP.ready('Vue 2 Wallpaper');
    
    AnyWP.onClick('.increment-btn', () => {
      this.count++;
    }, { waitFor: true });
  }
};
</script>
```

### Vue Router Navigation

```javascript
// router/index.js
import { createRouter } from 'vue-router';

const router = createRouter({...});

router.afterEach(() => {
  // Refresh bounds after route change
  if (window.AnyWP) {
    setTimeout(() => {
      AnyWP.refreshBounds();
    }, 500);
  }
});

export default router;
```

### Best Practices

1. **Initialize in root component**
   ```vue
   // App.vue
   onMounted(() => {
     if (window.AnyWP) {
       AnyWP.ready('My App');
       AnyWP.setSPAMode(true);
     }
   });
   ```

2. **Use `waitFor` option**
   ```javascript
   AnyWP.onClick('.my-button', callback, { waitFor: true });
   ```

3. **Avoid duplicate registration**
   ```javascript
   // ✅ Correct - register only in onMounted
   onMounted(() => {
     AnyWP.onClick('.btn', () => count.value++, { waitFor: true });
   });
   ```

---

## 🔄 SPA (Single Page Application) Support

### Auto Detection

The SDK automatically detects these frameworks:
- React
- Vue
- Angular

When SPA is detected, the following features are enabled automatically:
- ✅ Route change monitoring (pushState/replaceState)
- ✅ DOM mutation monitoring (MutationObserver)
- ✅ Auto refresh click bounds
- ✅ Element remount detection

### Manual Configuration

If auto-detection fails, enable manually:

```javascript
// On app initialization
AnyWP.setSPAMode(true);
```

### Route Change Handling

#### Method 1: Auto Handling (Recommended)

SDK automatically monitors route changes and refreshes bounds. Just use `waitFor`:

```javascript
AnyWP.onClick('.my-button', callback, { waitFor: true });
```

#### Method 2: Manual Refresh

Manually call after route change:

```javascript
// React Router
useEffect(() => {
  setTimeout(() => AnyWP.refreshBounds(), 500);
}, [location]);

// Vue Router
router.afterEach(() => {
  setTimeout(() => AnyWP.refreshBounds(), 500);
});
```

### Dynamic Content Handling

#### Auto Mode

Use `autoRefresh` option (enabled by default):

```javascript
AnyWP.onClick('.dynamic-element', callback, {
  waitFor: true,
  autoRefresh: true  // Auto monitor element size/position changes
});
```

#### Manual Mode

Manually refresh after content update:

```javascript
// After adding/removing elements
addNewElement();
setTimeout(() => AnyWP.refreshBounds(), 100);
```

---

## 🐛 Debugging Tips

### 1. Enable Debug Mode

```javascript
// Method 1: Enable in code
AnyWP.enableDebug();

// Method 2: URL parameter
// http://localhost:3000?debug
```

Debug mode will:
- Show red borders marking click areas
- Output detailed logs to console
- Display physical and CSS coordinates

### 2. Check if SDK is Loaded

```javascript
if (window.AnyWP) {
  console.log('SDK Version:', AnyWP.version);
  console.log('SPA Mode:', AnyWP._spaMode);
} else {
  console.error('AnyWP SDK not loaded!');
}
```

### 3. Verify Click Areas

```javascript
// Show debug border on registration
AnyWP.onClick('#button', callback, { debug: true });
```

### 4. View Registered Handlers

```javascript
console.log('Registered handlers:', AnyWP._clickHandlers.length);
AnyWP._clickHandlers.forEach((h, i) => {
  console.log(`Handler ${i}:`, h.element, h.bounds);
});
```

### 5. Test Click Coordinates

```javascript
// Manually trigger click test
AnyWP._handleClick(500, 300);
```

---

## 📐 Coordinate System

### Physical Pixels vs CSS Pixels

SDK internally uses **physical pixels** (accounting for DPI scaling), but developers don't need to worry - SDK auto-converts.

```javascript
console.log('DPI Scale:', AnyWP.dpiScale);        // e.g. 1.5x
console.log('CSS Width:', window.innerWidth);     // 1920px
console.log('Physical Width:', AnyWP.screenWidth); // 2880px (1920 * 1.5)
```

### Notes

1. **Just use CSS positioning and sizes**, SDK handles scaling automatically
2. **Avoid fixed coordinates**, always register elements via selectors
3. **Responsive design** works automatically (with `autoRefresh`)

---

## ⚡ Performance Optimization

### 1. Reduce Registration Count

```javascript
// ❌ Not good - register for each list item
items.forEach(item => {
  AnyWP.onClick(`#item-${item.id}`, callback);
});

// ✅ Better - register parent container, handle via coordinates
AnyWP.onClick('#item-list', (x, y) => {
  // Determine which child was clicked via coordinates
});
```

### 2. Disable Unnecessary Auto Refresh

```javascript
// For static elements, disable autoRefresh
AnyWP.onClick('.static-button', callback, {
  autoRefresh: false
});
```

### 3. Delayed Initialization

```javascript
// Wait for critical content to render before registering
setTimeout(() => {
  AnyWP.onClick('.late-element', callback);
}, 2000);
```

### 4. Cleanup Promptly

```javascript
// Cleanup on component unmount
onUnmounted(() => {
  AnyWP.clearHandlers();
});
```

---

## 🌐 Browser Compatibility

| Feature | Requirement |
|---------|-------------|
| Basic functionality | WebView2 (Chromium Edge) |
| MutationObserver | ✅ Full support |
| ResizeObserver | ✅ Full support |
| ES6+ syntax | ✅ Full support |

**Note**: SDK runs in WebView2 environment based on latest Chromium, supporting all modern web features.

---

## 📝 Complete Examples

See example files in `examples/` directory:

- `test_simple.html` - Basic HTML example
- `test_react.html` - React integration example
- `test_vue.html` - Vue integration example
- `test_api.html` - Complete API demonstration

---

## ❓ FAQ

### Q: Why aren't clicks working?

1. Check if element exists: `document.querySelector('#myButton')`
2. Enable debug mode to see bounds: `AnyWP.onClick('#myButton', callback, { debug: true })`
3. Verify registration timing: Use `{ waitFor: true }` for SPA

### Q: Clicks fail after SPA route change?

Use `waitFor` option or call `AnyWP.refreshBounds()` after route change.

### Q: How to use in iframe?

iframe click detection is not currently supported. Register in top-level page.

### Q: Can I detect drag, scroll, etc.?

Current version only supports click events. More interactions coming in future versions.

### Q: How to test during development?

1. Preview in browser (limited functionality, mainly for UI)
2. Use AnyWP Engine example app to load webpage
3. Enable debug mode to verify click areas

---

## 🔗 Related Resources

- [Quick Start](QUICK_START.md)
- [Technical Notes](TECHNICAL_NOTES.md)
- [API Bridge](API_BRIDGE.md)
- [Flutter Plugin Usage Guide](PACKAGE_USAGE_GUIDE_CN.md)

---

**Version**: 4.0.0  
**Last Updated**: 2025-11-03

