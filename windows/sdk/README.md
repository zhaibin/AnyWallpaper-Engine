# AnyWP SDK - Modular Architecture

## 📁 目录结构

```
sdk/
├── core/               # 核心模块
│   ├── AnyWP.js       # 核心对象定义
│   └── init.js        # 初始化逻辑
├── modules/           # 功能模块
│   ├── animations.js  # 动画控制
│   ├── click.js       # 点击处理
│   ├── drag.js        # 拖拽功能
│   ├── events.js      # 事件系统
│   ├── spa.js         # SPA框架支持
│   └── storage.js     # 状态持久化
├── utils/             # 工具模块
│   ├── bounds.js      # 边界计算
│   ├── coordinates.js # 坐标转换
│   └── debug.js       # 调试工具
├── index.js           # 入口文件
├── package.json       # 依赖配置
├── rollup.config.js   # 构建配置
└── README.md          # 本文档
```

## 🛠️ 开发流程

### 安装依赖

```bash
cd windows/sdk
npm install
```

### 构建

```bash
# 开发构建（未压缩）
npm run build

# 生产构建（压缩版）
npm run build:production

# 监听模式（自动重新构建）
npm run build:watch
```

### 构建输出

- `windows/anywp_sdk.js` - 标准版本（未压缩）
- `windows/anywp_sdk.min.js` - 压缩版本（仅生产构建）

## 📝 模块说明

### 核心模块 (core/)

**AnyWP.js**
- 核心对象定义
- 状态管理
- 公共属性

**init.js**
- 初始化逻辑
- 模块装配

### 功能模块 (modules/)

**events.js**
- 事件监听器管理
- 鼠标/键盘/可见性事件

**click.js**
- 点击区域注册
- 边界检测
- 自动刷新

**drag.js**
- 拖拽功能
- 位置持久化
- 边界限制

**storage.js**
- 状态保存/加载
- 原生存储桥接
- LocalStorage 回退

**spa.js**
- React/Vue/Angular 检测
- 路由变化监听
- DOM 变化监控

**animations.js**
- 动画暂停/恢复
- 视频/音频控制
- RequestAnimationFrame 拦截

### 工具模块 (utils/)

**bounds.js**
- 元素边界计算
- 碰撞检测

**coordinates.js**
- 坐标系转换
- 物理像素 ↔ CSS 像素

**debug.js**
- 调试日志
- 可视化边界
- URL 参数检测

## 🔄 工作流程

1. **修改源码** - 编辑 `sdk/` 目录下的模块文件
2. **构建** - 运行 `npm run build`
3. **测试** - 使用生成的 `anywp_sdk.js` 测试
4. **发布** - 生产构建 `npm run build:production`

## 📊 代码统计

| 模块 | 原始行数 | 优化后行数 | 减少比例 |
|------|---------|-----------|---------|
| 总计 | 1211行 | ~950行* | ~21% |

*预估，去除重复代码和优化后的行数

## 🎯 优势

✅ **模块化** - 清晰的职责划分  
✅ **可维护** - 快速定位和修改  
✅ **可测试** - 独立模块单元测试  
✅ **可扩展** - 轻松添加新功能  
✅ **向后兼容** - API 保持不变

## 📚 更多文档

- [Web开发者指南](../../docs/WEB_DEVELOPER_GUIDE_CN.md)
- [API使用示例](../../docs/API_USAGE_EXAMPLES.md)
- [技术笔记](../../docs/TECHNICAL_NOTES.md)

