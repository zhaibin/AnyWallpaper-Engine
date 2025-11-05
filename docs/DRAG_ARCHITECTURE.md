# 拖拽功能架构设计

## 🎯 设计目标

实现互动壁纸的拖拽功能，同时满足以下约束：
- ✅ 壁纸必须在桌面图标**下方**（WorkerW 层）
- ✅ 保持**鼠标透明**（WS_EX_TRANSPARENT），允许点击穿透到桌面
- ✅ 支持拖拽交互（需要捕获 mousedown, mousemove, mouseup）
- ✅ 状态持久化（重启后恢复位置）

## 🏗️ 架构方案

### 核心思路
使用 **C++ 低级鼠标钩子**（`SetWindowsHookEx` + `WH_MOUSE_LL`）在系统层面捕获鼠标事件，然后转发到 WebView2。

### 为什么不能禁用鼠标透明？
如果禁用鼠标透明（取消 `WS_EX_TRANSPARENT`）：
- ❌ 壁纸窗口会捕获所有鼠标事件
- ❌ 桌面图标无法点击
- ❌ 不符合"壁纸在桌面下方"的定义

### 为什么必须用鼠标钩子？
启用鼠标透明后：
- ❌ DOM 鼠标事件（onclick, onmousedown等）不会触发
- ✅ 鼠标钩子在系统层面捕获，不受窗口属性影响
- ✅ 可以选择性转发到 WebView2

## 📊 事件流程图

```
┌─────────────────────────────────────────────────────────────────┐
│ 用户操作                                                          │
│   鼠标移动/点击桌面                                                │
└───────────────────────┬─────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ Windows 系统层                                                    │
│   低级鼠标钩子 (WH_MOUSE_LL)                                      │
│   └─ LowLevelMouseProc() 捕获所有鼠标事件                         │
└───────────────────────┬─────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ C++ 插件逻辑 (anywp_engine_plugin.cpp)                            │
│                                                                   │
│   1. 检查鼠标位置是否被其他窗口遮挡                                │
│      if (WindowFromPoint() == 其他应用窗口)                        │
│         → 跳过，不转发                                            │
│                                                                   │
│   2. 判断事件类型                                                 │
│      - WM_LBUTTONDOWN  → "mousedown"                             │
│      - WM_MOUSEMOVE    → "mousemove"                             │
│      - WM_LBUTTONUP    → "mouseup"                               │
│                                                                   │
│   3. 转发到 WebView2                                              │
│      SendClickToWebView(x, y, event_type)                        │
└───────────────────────┬─────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ WebView2 (JavaScript 环境)                                        │
│                                                                   │
│   ExecuteScript: 派发 CustomEvent                                 │
│      window.dispatchEvent(                                        │
│        new CustomEvent('AnyWP:mouse', {                           │
│          detail: { type, x, y }                                   │
│        })                                                         │
│      )                                                            │
└───────────────────────┬─────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ JavaScript SDK (anywp_sdk.js)                                     │
│                                                                   │
│   window.addEventListener('AnyWP:mouse', handleGlobalMouse)       │
│                                                                   │
│   makeDraggable() 逻辑:                                           │
│   ├─ mousedown: 检查鼠标是否在元素上 → 开始拖拽                    │
│   ├─ mousemove: 如果正在拖拽 → 更新元素位置                        │
│   └─ mouseup:   如果正在拖拽 → 结束拖拽 + 保存状态                 │
└───────────────────────┬─────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────────┐
│ 状态持久化 (Windows Registry)                                     │
│                                                                   │
│   SaveState(key, value)                                           │
│   └─ RegSetValueExA(                                              │
│        HKEY_CURRENT_USER\Software\AnyWPEngine\State,              │
│        key, JSON.stringify({left, top})                           │
│      )                                                            │
└─────────────────────────────────────────────────────────────────┘
```

## 🔑 关键实现

### 1. C++ 鼠标钩子 (anywp_engine_plugin.cpp)

```cpp
// Line 1548: 鼠标钩子回调
LRESULT CALLBACK AnyWPEnginePlugin::LowLevelMouseProc(
    int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0 && hook_instance_) {
    MSLLHOOKSTRUCT* info = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    POINT pt = info->pt;
    
    // 检查是否被其他应用窗口遮挡
    HWND window_at_point = WindowFromPoint(pt);
    bool is_app_window = /* 判断是否为应用窗口 */;
    
    if (is_app_window) {
      return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }
    
    // 转发鼠标事件
    const char* event_type = nullptr;
    if (wParam == WM_LBUTTONDOWN) event_type = "mousedown";
    else if (wParam == WM_MOUSEMOVE) event_type = "mousemove";
    else if (wParam == WM_LBUTTONUP) event_type = "mouseup";
    
    if (event_type) {
      hook_instance_->SendClickToWebView(pt.x, pt.y, event_type);
    }
  }
  
  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// Line 1649: 转发事件到 WebView2
void AnyWPEnginePlugin::SendClickToWebView(
    int x, int y, const char* event_type) {
  std::wstringstream script;
  script << L"window.dispatchEvent(new CustomEvent('AnyWP:mouse', {"
         << L"  detail: { type: '" << event_type << L"', "
         << L"            x: " << x << L", y: " << y << L" }"
         << L"}));";
  
  target_webview->ExecuteScript(script.str().c_str(), nullptr);
}
```

### 2. JavaScript SDK 拖拽实现 (anywp_sdk.js)

```javascript
// Line 622: 基于全局鼠标事件的拖拽
function handleGlobalMouse(event) {
  if (!self.interactionEnabled) return;
  
  const detail = event.detail;
  const mouseX = detail.x;  // 物理像素坐标
  const mouseY = detail.y;
  const mouseType = detail.type;
  
  const rect = el.getBoundingClientRect();
  const dpi = self.dpiScale;
  
  // 转换为物理像素
  const physicalLeft = Math.round(rect.left * dpi);
  const physicalTop = Math.round(rect.top * dpi);
  const physicalRight = Math.round(rect.right * dpi);
  const physicalBottom = Math.round(rect.bottom * dpi);
  
  // 检查鼠标是否在元素上
  const isOver = mouseX >= physicalLeft && mouseX <= physicalRight &&
                 mouseY >= physicalTop && mouseY <= physicalBottom;
  
  if (mouseType === 'mousedown' && isOver) {
    // 开始拖拽
    self._dragState = {
      element: el,
      startX: mouseX,
      startY: mouseY,
      offsetX: mouseX - physicalLeft,
      offsetY: mouseY - physicalTop
    };
  }
  else if (mouseType === 'mousemove' && self._dragState) {
    // 拖拽中
    let newPhysicalLeft = mouseX - self._dragState.offsetX;
    let newPhysicalTop = mouseY - self._dragState.offsetY;
    
    // 转换回 CSS 像素
    el.style.left = (newPhysicalLeft / dpi) + 'px';
    el.style.top = (newPhysicalTop / dpi) + 'px';
  }
  else if (mouseType === 'mouseup' && self._dragState) {
    // 结束拖拽，保存位置
    self._saveElementPosition(persistKey, finalPos.x, finalPos.y);
    self._dragState = null;
  }
}

window.addEventListener('AnyWP:mouse', handleGlobalMouse);
```

### 3. 状态持久化 (C++ + JavaScript)

```cpp
// C++ 保存到 Registry
bool AnyWPEnginePlugin::SaveState(
    const std::string& key, const std::string& value) {
  HKEY hKey;
  RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\AnyWPEngine\\State", ...);
  RegSetValueExA(hKey, key.c_str(), 0, REG_SZ, value.c_str(), ...);
  RegCloseKey(hKey);
  return true;
}
```

```javascript
// JavaScript 保存位置
_saveElementPosition: function(key, x, y) {
  const position = { left: x, top: y };
  
  // 发送到 C++ 层
  window.chrome.webview.postMessage({
    type: 'saveState',
    key: key,
    value: JSON.stringify(position)
  });
}
```

## ⚡ 性能优化

### 1. mousemove 事件优化
问题：mousemove 事件频率极高（每秒数百次），会影响性能

解决方案：
- 只在拖拽状态时处理 mousemove（检查 `self._dragState`）
- 不拖拽时，mousemove 事件虽然转发但被忽略
- 未来可考虑：只在 mousedown 后才启用 mousemove 转发

### 2. 坐标转换
- C++ 传递的是物理像素（考虑 DPI 缩放）
- JavaScript 需要转换为 CSS 像素
- 使用 `window.devicePixelRatio` 进行转换

### 3. 窗口遮挡检测
- 使用 `WindowFromPoint()` 快速检测
- 如果被应用窗口遮挡，不转发事件（减少开销）

## 🎨 用户体验

### 优点
✅ 真正的壁纸体验（在图标下方）
✅ 支持拖拽交互
✅ 不在元素上时，点击穿透到桌面
✅ 状态持久化，重启后恢复

### 限制
⚠️ mousemove 事件可能有轻微延迟（鼠标钩子 → WebView2）
⚠️ 不支持右键菜单（可以扩展）
⚠️ 拖拽时鼠标样式无法自定义（透明窗口限制）

## 🔮 未来改进

1. **条件启用 mousemove**
   - 只在 mousedown 后才转发 mousemove
   - mouseup 后停止转发
   - 进一步减少性能开销

2. **多点触控支持**
   - 扩展鼠标钩子支持触摸事件
   - 平板/触摸屏支持

3. **手势识别**
   - 在 JavaScript 层实现手势识别
   - 滑动、缩放等

4. **性能监控**
   - 监控鼠标钩子开销
   - 自适应调整事件转发频率

## 📚 相关文件

- `windows/anywp_engine_plugin.cpp` - C++ 鼠标钩子实现
- `windows/anywp_sdk.js` - JavaScript SDK 拖拽逻辑
- `examples/test_draggable.html` - 拖拽功能演示
- `examples/README_DRAG_TEST.md` - 使用指南
- `CHANGELOG_CN.md` - 更新日志

## 🏆 技术亮点

这个架构的核心价值在于：

1. **保持真实壁纸特性**：窗口透明，在桌面图标下方
2. **支持复杂交互**：通过系统级钩子突破窗口透明限制
3. **状态持久化**：用户体验的连续性
4. **DPI 感知**：正确处理高 DPI 显示器

这是一个**系统级与 Web 技术结合**的优秀案例！

