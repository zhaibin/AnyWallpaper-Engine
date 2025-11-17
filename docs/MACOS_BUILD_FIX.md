# macOS Build Fix Guide

## 问题描述

在 Flutter 3.38.1 + Xcode 26.1.1 + CocoaPods 1.16.2 环境下，macOS 插件构建失败，报错：
```
fatal error: 'FlutterMacOS/FlutterMacOS.h' file not found
error: Unable to find module dependency: 'FlutterMacOS'
```

## 根本原因

1. **FlutterMacOS 框架路径缺失**：CocoaPods 为插件 pod 设置的框架搜索路径不包含 FlutterMacOS 框架
2. **模块映射文件路径缺失**：Clang 模块系统无法找到 FlutterMacOS 的模块映射文件
3. **Runner target 框架路径缺失**：Runner 应用编译时无法解析插件模块的 FlutterMacOS 依赖

## 修复方案

### 1. 修改 Podfile（插件级别）

在 `example/macos/Podfile` 的 `post_install` 钩子中添加：

```ruby
post_install do |installer|
  installer.pods_project.targets.each do |target|
    flutter_additional_macos_build_settings(target)
    
    # Fix framework search paths for plugin pods that depend on FlutterMacOS
    if target.name == 'anywp_engine'
      target.build_configurations.each do |config|
        # Use the same method as flutter_additional_macos_build_settings
        artifacts_dir = File.join(flutter_root, 'bin', 'cache', 'artifacts', 'engine')
        debug_framework_dir = File.join(artifacts_dir, 'darwin-x64', 'FlutterMacOS.xcframework')
        release_framework_dir = File.join(artifacts_dir, 'darwin-x64-release', 'FlutterMacOS.xcframework')
        
        configuration_engine_dir = (config.type == :debug ? debug_framework_dir : release_framework_dir)
        
        if Dir.exist?(configuration_engine_dir)
          Dir.new(configuration_engine_dir).each_child do |xcframework_file|
            next if xcframework_file.start_with?('.')
            if xcframework_file.start_with?('macos-')
              framework_path = File.join(configuration_engine_dir, xcframework_file)
              framework_full_path = File.join(framework_path, 'FlutterMacOS.framework')
              header_path = File.join(framework_full_path, 'Headers')
              modulemap_path = File.join(framework_full_path, 'Modules', 'module.modulemap')
              
              # Override with correct absolute path
              config.build_settings['FRAMEWORK_SEARCH_PATHS'] = "\"#{framework_path}\" $(inherited)"
              config.build_settings['HEADER_SEARCH_PATHS'] = "\"#{header_path}\" $(inherited)"
              
              # Add module map file if it exists
              if File.exist?(modulemap_path)
                existing_cflags = config.build_settings['OTHER_CFLAGS'] || '$(inherited)'
                unless existing_cflags.include?('-fmodule-map-file')
                  config.build_settings['OTHER_CFLAGS'] = "#{existing_cflags} -fmodule-map-file=\"#{modulemap_path}\""
                end
              end
              
              puts "[Podfile] Fixed framework path for #{target.name} (#{config.name}): #{framework_path}"
              break
            end
          end
        end
      end
      
      # Also fix xcconfig files directly (CocoaPods sometimes doesn't pick up post_install changes)
      installer.pods_project.targets.each do |pod_target|
        if pod_target.name == 'anywp_engine'
          pod_target.build_configurations.each do |config|
            xcconfig_path = File.join(installer.sandbox.root, 'Target Support Files', pod_target.name, "#{pod_target.name}.#{config.name.downcase}.xcconfig")
            if File.exist?(xcconfig_path)
              xcconfig_content = File.read(xcconfig_path)
              artifacts_dir = File.join(flutter_root, 'bin', 'cache', 'artifacts', 'engine')
              debug_framework_dir = File.join(artifacts_dir, 'darwin-x64', 'FlutterMacOS.xcframework')
              release_framework_dir = File.join(artifacts_dir, 'darwin-x64-release', 'FlutterMacOS.xcframework')
              configuration_engine_dir = (config.type == :debug ? debug_framework_dir : release_framework_dir)
              
              if Dir.exist?(configuration_engine_dir)
                Dir.new(configuration_engine_dir).each_child do |xcframework_file|
                  next if xcframework_file.start_with?('.')
                  if xcframework_file.start_with?('macos-')
                    framework_path = File.join(configuration_engine_dir, xcframework_file, 'FlutterMacOS.framework')
                    header_path = File.join(framework_path, 'Headers')
                    modulemap_path = File.join(framework_path, 'Modules', 'module.modulemap')
                    
                    # Remove existing incorrect paths
                    xcconfig_content.gsub!(/^FRAMEWORK_SEARCH_PATHS.*$/m, '')
                    xcconfig_content.gsub!(/^HEADER_SEARCH_PATHS.*$/m, '')
                    xcconfig_content.gsub!(/^SWIFT_INCLUDE_PATHS.*$/m, '')
                    
                    # Add correct paths
                    xcconfig_content += "\nFRAMEWORK_SEARCH_PATHS = \"#{File.dirname(framework_path)}\" $(inherited)\n"
                    xcconfig_content += "HEADER_SEARCH_PATHS = \"#{header_path}\" $(inherited)\n"
                    if File.exist?(modulemap_path)
                      xcconfig_content += "SWIFT_INCLUDE_PATHS = \"#{File.dirname(modulemap_path)}\" $(inherited)\n"
                      xcconfig_content += "OTHER_CFLAGS = $(inherited) -fmodule-map-file=\"#{modulemap_path}\"\n"
                    end
                    
                    File.write(xcconfig_path, xcconfig_content)
                    puts "[Podfile] Fixed xcconfig for #{pod_target.name} (#{config.name})"
                    break
                  end
                end
              end
            end
          end
        end
      end
    end
  end
end
```

### 2. 修改 Runner 配置（应用级别）

在 `example/macos/Runner/Configs/Debug.xcconfig` 和 `Release.xcconfig` 中添加：

```xcconfig
// Add FlutterMacOS framework search path for module resolution
FRAMEWORK_SEARCH_PATHS = $(inherited) "$(FLUTTER_ROOT)/bin/cache/artifacts/engine/darwin-x64/FlutterMacOS.xcframework/macos-arm64_x86_64"
HEADER_SEARCH_PATHS = $(inherited) "$(FLUTTER_ROOT)/bin/cache/artifacts/engine/darwin-x64/FlutterMacOS.xcframework/macos-arm64_x86_64/FlutterMacOS.framework/Headers"
OTHER_CFLAGS = $(inherited) -fmodule-map-file="$(FLUTTER_ROOT)/bin/cache/artifacts/engine/darwin-x64/FlutterMacOS.xcframework/macos-arm64_x86_64/FlutterMacOS.framework/Modules/module.modulemap"
```

**注意**：Release 配置使用 `darwin-x64-release` 目录。

### 3. 修复其他 Xcode 项目配置

#### 3.1 修复 project.pbxproj

在 `example/macos/Runner.xcodeproj/project.pbxproj` 中：

1. 添加 `SWIFT_VERSION = 5.0;` 到 Debug 和 Release 配置
2. 添加 `INFOPLIST_FILE = Runner/Info.plist;` 到构建设置
3. 修复 entitlements 文件路径
4. 修复 `GeneratedPluginRegistrant.swift` 和 `Assets.xcassets` 路径
5. 添加缺失的 `Debug.xcconfig` 和 `Release.xcconfig` 文件引用

#### 3.2 修复 .xcconfig 文件

在 `Runner/Configs/Debug.xcconfig` 和 `Release.xcconfig` 中：

1. 添加 CocoaPods 生成的配置引用：
```xcconfig
#include "Pods/Target Support Files/Pods-Runner/Pods-Runner.debug.xcconfig"
```

### 4. 修复 AnyWPEnginePlugin.m 警告

添加 `__unused` 限定符：

```objective-c
- (void)handleEncryptFile:(FlutterMethodCall*)call result:(FlutterResult)result {
    NSDictionary *args = call.arguments;
    __unused NSString *sourcePath = args[@"sourcePath"];
    __unused NSString *destPath = args[@"destPath"];
    // ... implementation ...
}
```

## 验证步骤

```bash
cd example
flutter clean
flutter pub get
cd macos
pod install
cd ..
flutter build macos --debug
flutter build macos --release
```

## 环境信息

- **Flutter**: 3.38.1 (stable)
- **Dart**: 3.10.0
- **Xcode**: 26.1.1 (Build 17B100)
- **CocoaPods**: 1.16.2
- **macOS**: 26.1 (25B78)

## 参考

- Flutter macOS Plugin Development: https://flutter.dev/docs/development/platform-integration/platform-channels
- CocoaPods Podfile DSL: https://guides.cocoapods.org/syntax/podfile.html
- Xcode Build Settings: https://developer.apple.com/documentation/xcode/build-settings-reference

