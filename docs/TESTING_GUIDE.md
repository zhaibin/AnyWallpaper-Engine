# AnyWP Engine 测试指南

## 📋 测试概览

AnyWP Engine 项目包含三个层次的测试：

### 1. 单元测试（C++）

**位置**: `windows/test/`  
**运行**: `cd windows\test && run_tests.bat`  
**覆盖**: C++ 工具类和模块

### 2. Web SDK 测试（TypeScript）

**位置**: `sdk/src/__tests__/`  
**运行**: `cd sdk\src && npm test`  
**覆盖**: JavaScript SDK API

### 3. 预编译包测试（PowerShell）⭐ **新增**

**位置**: `scripts/test_precompiled_package.ps1`  
**运行**: `.\scripts\test_precompiled_package.ps1 -Version "2.4.1" -TestLevel Quick`  
**覆盖**: 预编译包完整性和功能验证

详见: [预编译包测试指南](PRECOMPILED_PACKAGE_TESTING.md)

## 🎯 测试策略

### 开发时测试

```powershell
# C++ 代码修改后
cd windows\test
run_tests.bat

# Web SDK 修改后
cd sdk\src
npm test
```

### 发布前测试

```powershell
# 1. 构建预编译包
.\scripts\release.bat

# 2. 快速验证包完整性（推荐）⭐
.\scripts\test_precompiled_package.ps1 -Version "2.4.1" -TestLevel Quick

# 3. 完整集成测试（可选）
.\scripts\test_full.bat
```

### CI/CD 集成

```yaml
# GitHub Actions 示例
- name: Run Package Tests
  run: |
    .\scripts\test_precompiled_package.ps1 -Version "2.4.1" -TestLevel Quick
```

## 📊 测试覆盖率

| 模块 | 覆盖率目标 | 当前覆盖率 |
|------|-----------|----------|
| C++ Utils | ≥95% | ~95% |
| C++ Modules | ≥90% | ~90% |
| Web SDK | ≥90% | ~85% |
| 预编译包 | 100% | 100% |

## 🔍 测试类型对比

| 测试类型 | 速度 | 范围 | 适用场景 |
|---------|------|------|---------|
| **C++ 单元测试** | 快（<10s） | 工具类/模块 | 开发中 |
| **Web SDK 测试** | 快（<5s） | JS API | SDK开发 |
| **预编译包测试-Basic** | 快（<5s） | 文件完整性 | 每次打包 |
| **预编译包测试-Quick** | 中（<30s） | API+功能 | 发布前 |
| **预编译包测试-Full** | 慢（<2min） | 完整功能 | 重要发布 |
| **集成测试** | 慢（<3min） | 端到端 | 重大变更 |

## 📝 测试最佳实践

### 1. 新功能开发

```
开发 → C++单元测试 → Web SDK测试 → 本地运行验证
```

### 2. Bug 修复

```
重现Bug → 添加失败测试 → 修复代码 → 验证测试通过
```

### 3. 发布流程

```
更新版本 → 构建Release → 预编译包测试Quick → 集成测试（可选） → 发布
```

## ⚠️ 重要提醒

1. **所有测试必须在发布前通过** ✓
2. **新功能必须添加测试用例** ✓
3. **测试覆盖率不得降低** ✓
4. **CI失败不得合并代码** ✓

## 📚 相关文档

- [预编译包测试详细指南](PRECOMPILED_PACKAGE_TESTING.md)
- [C++ 单元测试框架](../windows/test/README.md)
- [Web SDK 测试](../sdk/src/README.md)
- [开发者API参考](DEVELOPER_API_REFERENCE.md)

---

**最后更新**: 2025-11-19  
**文档版本**: 1.0.0
