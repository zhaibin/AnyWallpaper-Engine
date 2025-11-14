# AnyWP Engine v2.1.5 - Release Notes

**发布日期**: 2025-01-15
**版本**: 2.1.5

---

## 🚀 发版流程自动化改进


## 🚀 发版流程自动化改进

### 新增自动化工具
- **版本一致性检查**: `scripts/check_version_consistency.ps1` - 自动检查 `pubspec.yaml`、`CHANGELOG_CN.md`、`.cursorrules`、`docs/PRECOMPILED_DLL_INTEGRATION.md` 的版本号一致性
- **GitHub Release Notes 自动生成**: `scripts/generate_release_notes.ps1` - 从 CHANGELOG 自动生成 GitHub Release Notes
- **Git 提交模板自动生成**: `scripts/generate_commit_template.ps1` - 自动生成中文提交信息模板
- **Git 提交自动化**: `scripts/release_git.bat` - 自动执行 Git 提交、Tag 创建和推送
- **Web SDK 包验证**: `scripts/verify_precompiled.bat` 扩展支持 Web SDK 包验证

### 改进发布脚本
- **release.bat 增强**: 
  - 自动运行版本一致性检查（构建前）
  - 自动生成 GitHub Release Notes（`release/GITHUB_RELEASE_NOTES_v{版本号}.md`）
  - 自动生成 Git 提交模板（`release/commit_msg_v{版本号}.txt`）
  - 统一步骤计数和进度显示（30 步）

### 文档更新
- **.cursorrules**: 更新发版流程文档，包含自动化工具说明
- **docs/RELEASE_GUIDE.md**: 更新为使用新的自动化脚本

## 📝 代码变更

- `scripts/release_utils.psm1`: 新增 PowerShell 模块，封装版本号读取和 Changelog 解析
- `scripts/generate_release_notes.ps1`: 新增 GitHub Release Notes 生成脚本
- `scripts/generate_commit_template.ps1`: 新增 Git 提交模板生成脚本
- `scripts/check_version_consistency.ps1`: 新增版本一致性检查脚本
- `scripts/release_git.bat`: 新增 Git 提交自动化脚本（提交、Tag、推送）
- `scripts/release.bat`: 集成自动化工具，添加版本检查和文档生成步骤，完善后续步骤提示
- `scripts/verify_precompiled.bat`: 扩展支持 Web SDK 包验证（3/3）
- `.cursorrules`: 更新发版流程文档，添加 Git 自动化脚本说明
- `docs/RELEASE_GUIDE.md`: 更新发布指南

## ✅ 改进效果

- ✅ 版本号一致性自动检查，避免手动遗漏
- ✅ GitHub Release Notes 自动生成，无需手动编写
- ✅ Git 提交模板自动生成，统一提交格式
- ✅ Git 提交和 Tag 创建自动化（可选，支持 `--no-push` 参数）
- ✅ Web SDK 包验证完整，确保三类包都正确
- ✅ 发版流程更加标准化和自动化，从构建到 Git 推送全流程自动化

---

