# AnyWP Engine 示例（最小集成）

这是一个最小可运行的 Flutter 桌面项目，演示如何在应用中集成 AnyWP Engine 预编译包。

## 使用步骤

1. 确保已经按照 `setup_precompiled.bat` 安装 AnyWP Engine 预编译包。
2. 在该示例目录执行：

   ```bash
   flutter pub get
   flutter run -d windows
   ```

3. 点击页面上的 **“启动壁纸”** 按钮，即可在桌面上加载指定的网页。

> ⚠️ 注意：示例依赖路径默认指向父目录 `..`，请确保示例与预编译包位于同一目录。

