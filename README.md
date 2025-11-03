# AnyWP Engine

A Flutter Windows plugin that embeds WebView2 as an interactive desktop wallpaper, displaying web content behind desktop icons.

> **Note about Project Folder**: The project folder may be named `AnyWallpaper-Engine` (with dash). We recommend renaming to `AnyWP_Engine` for consistency with code naming. See [NAMING_CONVENTION.md](NAMING_CONVENTION.md) for details.

## ✨ Features

- 🖼️ **WebView2 Integration** - Display any web content as desktop wallpaper
- 🎯 **Proper Z-Order** - WebView renders behind desktop icons (not covering them)
- 🖱️ **Mouse Transparency** - Optional click-through to desktop
- 📺 **Multi-Resolution** - Tested on 5120x2784 displays
- 🪟 **Windows 11 Support** - Optimized for Windows 11 desktop architecture
- ⚡ **Performance** - Hardware-accelerated web rendering

## 🚀 Quick Start

### Installation

**📦 Want to use this in your own project?**  
👉 See **[Quick Integration Guide](QUICK_INTEGRATION.md)** for 3 ways to integrate  
👉 See **[Complete Package Usage Guide](docs/PACKAGE_USAGE_GUIDE_CN.md)** for detailed documentation

Add to your `pubspec.yaml`:

```yaml
dependencies:
  anywp_engine:
    path: ../
```

### Usage

```dart
import 'package:anywp_engine/anywp_engine.dart';

// Start wallpaper (with mouse transparency)
await AnyWPEngine.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,
);

// Interactive wallpaper (clickable)
await AnyWPEngine.initializeWallpaper(
  url: 'https://game.example.com',
  enableMouseTransparent: false,
);

// Stop wallpaper
await AnyWPEngine.stopWallpaper();

// Navigate to different URL
await AnyWPEngine.navigateToUrl('https://new-url.com');
```

## 🛠️ Setup

### Prerequisites

- Windows 10/11
- Flutter 3.0+
- Visual Studio 2022 Build Tools
- WebView2 Runtime (included in Windows 11)

### Build

```bash
# Install WebView2 SDK (first time only)
.\scripts\setup_webview2.bat

# Build and run
.\scripts\build_and_run.bat
```

## 🏗️ Technical Architecture

### Core Implementation

The plugin uses a sophisticated approach to place WebView2 in the correct layer:

1. **Find Progman window** - The desktop's root window
2. **Create WS_CHILD window** - As a child of Progman
3. **Set Z-Order** - Position behind SHELLDLL_DefView (icon layer)
4. **Initialize WebView2** - Embed browser engine

```cpp
// Simplified core logic
HWND progman = FindWindowW(L"Progman", nullptr);
HWND host = CreateWindowExW(
    WS_EX_NOACTIVATE,
    L"STATIC", L"WebView2Host",
    WS_CHILD | WS_VISIBLE,
    0, 0, width, height,
    progman, nullptr, nullptr, nullptr
);

// Position behind desktop icons
HWND shelldll = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
SetWindowPos(host, shelldll, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
```

### Window Hierarchy

```
Progman (Desktop Window)
 ├─ SHELLDLL_DefView (Desktop Icons) - Z-Order: Front
 └─ WebView2 Host (AnyWP Window)     - Z-Order: Back
     └─ WebView2 Controller
         └─ Browser Content
```

## 📖 Documentation

### User Guides
- [中文文档](docs/README_CN.md) - Chinese documentation
- [快速开始](docs/QUICK_START.md) - Quick start guide
- [测试指南](docs/TESTING_GUIDE.md) - Testing guide
- [Usage Examples](docs/USAGE_EXAMPLES.md) - Code examples
- [Troubleshooting](docs/TROUBLESHOOTING.md) - Common issues

### 📦 Integration & Packaging
- [⚡ Quick Integration](QUICK_INTEGRATION.md) - 30-second integration guide
- [📚 Package Usage Guide](docs/PACKAGE_USAGE_GUIDE_CN.md) - Complete packaging guide (中文)
- [🏗️ Integration Architecture](docs/INTEGRATION_ARCHITECTURE.md) - Architecture & workflows
- [🎯 Cheat Sheet](docs/CHEAT_SHEET_CN.md) - Quick reference card (中文)

### Technical Guides
- [Technical Notes](docs/TECHNICAL_NOTES.md) - Implementation details
- [API Bridge](docs/API_BRIDGE.md) - JavaScript Bridge documentation
- [Project Structure](docs/PROJECT_STRUCTURE.md) - Project organization

## 🎮 Example App

The example app provides a full-featured control panel:

- URL input with presets
- Mouse transparency toggle
- Start/Stop controls
- Status indicators

## 🔧 Configuration

### Mouse Transparency

**Enabled** (default):
- Clicks pass through to desktop
- Desktop icons remain clickable
- WebView content not interactive

**Disabled**:
- Can interact with web content
- Useful for games, interactive dashboards
- Desktop icons may be obscured

### Window Size

Automatically uses work area (excludes taskbar):
```cpp
SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
```

## 🧪 Testing

测试文件位于 `examples/` 目录：
- `test_simple.html` - 简单测试页面
- `test_api.html` - API 功能测试
- `test_iframe_ads.html` - iframe 测试

运行测试：
```bash
# 自动测试
.\scripts\test.bat

# 手动运行
.\scripts\run.bat
```

Tested on:
- ✅ Windows 11 (Build 22000+)
- ✅ 5120x2784 resolution
- ✅ Multiple WorkerW configurations
- ✅ Various web content types

## 🤝 Contributing

Contributions welcome! Please read our contributing guidelines.

## 📝 License

MIT License - see [LICENSE](LICENSE) file

## 🙏 Acknowledgments

- Inspired by Wallpaper Engine and Lively Wallpaper
- Uses Microsoft Edge WebView2
- Built with Flutter

## 📞 Support

- GitHub Issues: [Report a bug](https://github.com/zhaibin/AnyWallpaper-Engine/issues)
- Discussions: [Ask questions](https://github.com/zhaibin/AnyWallpaper-Engine/discussions)

## 🗺️ Roadmap

- [ ] Multi-monitor support
- [ ] Performance profiling tools
- [ ] Custom transparency levels
- [ ] Video wallpaper presets
- [ ] Configuration file support
- [ ] System tray integration

---

**Made with ❤️ using Flutter and WebView2**
