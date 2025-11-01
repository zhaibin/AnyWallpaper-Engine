# GitHub 推送指南

## ✅ Git 本地仓库已创建

初始提交已完成：
- **提交数**: 1 个
- **文件数**: 44 个文件
- **代码行数**: 4393+ 行
- **分支**: master

## 📤 推送到 GitHub

### 方法 1: 创建新的 GitHub 仓库（推荐）

#### 1. 在 GitHub 上创建仓库

访问: https://github.com/new

填写信息：
- **Repository name**: `anywallpaper_engine`
- **Description**: Windows Desktop Wallpaper Engine with WebView2
- **Public/Private**: 选择你想要的可见性
- **⚠️ 不要勾选**: "Initialize with README" (我们已经有了)

#### 2. 连接并推送

创建仓库后，GitHub 会显示推送指令。在本地运行：

```bash
# 添加远程仓库（替换 YOUR_USERNAME）
git remote add origin https://github.com/YOUR_USERNAME/anywallpaper_engine.git

# 推送到 GitHub
git push -u origin master
```

**或者使用 SSH**（需要先配置 SSH key）：
```bash
git remote add origin git@github.com:YOUR_USERNAME/anywallpaper_engine.git
git push -u origin master
```

### 方法 2: 推送到已存在的仓库

如果你已经有仓库：

```bash
# 添加远程仓库
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git

# 推送（如果远程已有内容，使用 --force 强制推送）
git push -u origin master --force
```

## 🔐 GitHub 认证

### HTTPS 认证

如果使用 HTTPS，推送时会提示输入：
- **Username**: 你的 GitHub 用户名
- **Password**: 使用 **Personal Access Token**（不是账户密码）

#### 创建 Personal Access Token:

1. GitHub Settings → Developer settings → Personal access tokens → Tokens (classic)
2. 点击 "Generate new token"
3. 勾选 `repo` 权限
4. 生成并复制 token
5. 推送时使用 token 作为密码

### SSH 认证（推荐）

1. 生成 SSH key（如果还没有）：
   ```bash
   ssh-keygen -t ed25519 -C "your_email@example.com"
   ```

2. 添加到 GitHub：
   - 复制 `~/.ssh/id_ed25519.pub` 内容
   - GitHub Settings → SSH and GPG keys → New SSH key
   - 粘贴公钥

3. 测试连接：
   ```bash
   ssh -T git@github.com
   ```

## 📝 后续推送

完成首次推送后，后续更新只需：

```bash
# 添加更改
git add .

# 提交
git commit -m "描述你的更改"

# 推送
git push
```

## 🏷️ 创建 Release（可选）

1. 在 GitHub 仓库页面点击 "Releases"
2. 点击 "Create a new release"
3. 创建 tag: `v1.0.0`
4. 填写 Release notes
5. 上传编译好的可执行文件（Release 版本）

## 📋 推荐的仓库设置

### Topics（标签）

在仓库页面添加 topics：
- `flutter`
- `windows`
- `webview2`
- `desktop-wallpaper`
- `wallpaper-engine`
- `flutter-plugin`

### About 描述

```
Windows Desktop Wallpaper Engine with WebView2 - Display web content as interactive wallpaper behind desktop icons
```

### Website

可以添加：
- 文档网站
- Demo 视频链接

## 🔗 快速推送脚本

保存为 `push_to_github.bat`:

```batch
@echo off
echo Adding remote repository...
set /p GITHUB_URL="Enter your GitHub repository URL: "
git remote add origin %GITHUB_URL%

echo Pushing to GitHub...
git push -u origin master

echo Done!
pause
```

## ⚠️ 注意事项

1. **不要推送**:
   - 构建产物 (build/)
   - 依赖包 (packages/)
   - 临时文件
   - （已在 .gitignore 中排除）

2. **推送前检查**:
   ```bash
   git status
   git log --oneline
   ```

3. **敏感信息**:
   - 不要提交 API keys
   - 不要提交密码或 tokens

## 📊 当前状态

```bash
分支: master
提交: 1 个
文件: 44 个
代码: 4393+ 行
状态: ✅ 准备推送
```

## 🆘 常见问题

### 推送被拒绝

```bash
git push --force origin master
```

### 忘记用户名/密码

使用 Personal Access Token，不是账户密码

### SSL 证书错误

```bash
git config --global http.sslVerify false
```

---

**准备就绪！** 现在你可以将代码推送到 GitHub 了 🚀

