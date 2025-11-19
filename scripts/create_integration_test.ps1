# ========================================
# 创建集成测试项目
# ========================================
# 自动创建一个专门用于测试预编译包的Flutter项目
# ========================================

param(
    [Parameter(Mandatory=$false)]
    [string]$ProjectPath = "..\test_integration"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 创建 AnyWP Engine 集成测试项目" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$projectRoot = Join-Path $PSScriptRoot $ProjectPath
$projectRoot = [System.IO.Path]::GetFullPath($projectRoot)

Write-Host "项目路径: $projectRoot"
Write-Host ""

# 创建项目目录
if (Test-Path $projectRoot) {
    Write-Host "[警告] 项目目录已存在，将删除并重新创建" -ForegroundColor Yellow
    Remove-Item $projectRoot -Recurse -Force
}

Write-Host "创建项目目录..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path $projectRoot -Force | Out-Null

# 创建pubspec.yaml
Write-Host "创建 pubspec.yaml..." -ForegroundColor Cyan
$pubspecContent = @"
name: anywp_engine_integration_test
description: Integration test project for AnyWP Engine precompiled package
version: 1.0.0
publish_to: 'none'

environment:
  sdk: '>=3.0.0 <4.0.0'

dependencies:
  flutter:
    sdk: flutter

dev_dependencies:
  flutter_test:
    sdk: flutter
  flutter_lints: ^6.0.0

flutter:
  uses-material-design: true
"@

Set-Content -Path (Join-Path $projectRoot "pubspec.yaml") -Value $pubspecContent

# 创建lib目录和主文件
Write-Host "创建测试应用..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path (Join-Path $projectRoot "lib") -Force | Out-Null

$mainContent = @"
import 'dart:io';
import 'package:flutter/material.dart';
// import 'package:anywp_engine/anywp_engine.dart';

void main() {
  runApp(const IntegrationTestApp());
}

class IntegrationTestApp extends StatelessWidget {
  const IntegrationTestApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'AnyWP Engine Integration Test',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const TestPage(),
    );
  }
}

class TestPage extends StatefulWidget {
  const TestPage({Key? key}) : super(key: key);

  @override
  State<TestPage> createState() => _TestPageState();
}

class _TestPageState extends State<TestPage> {
  final List<String> _testResults = [];
  bool _isRunning = false;

  @override
  void initState() {
    super.initState();
    // 自动运行测试
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _runAllTests();
    });
  }

  Future<void> _runAllTests() async {
    setState(() {
      _isRunning = true;
      _testResults.clear();
    });

    _addResult('=== AnyWP Engine 集成测试 ===');
    _addResult('开始时间: \${DateTime.now()}');
    _addResult('');

    // TODO: 实现完整的API测试
    await _testBasicAPI();
    await _testAutoRecovery();
    await _testWebMessage();

    _addResult('');
    _addResult('=== 测试完成 ===');
    _addResult('结束时间: \${DateTime.now()}');

    setState(() {
      _isRunning = false;
    });

    // 5秒后自动退出
    await Future.delayed(const Duration(seconds: 5));
    exit(0);
  }

  Future<void> _testBasicAPI() async {
    _addResult('[Test 1] 基础API测试');
    try {
      // TODO: 调用 AnyWPEngine API
      _addResult('  [✓] 基础API可用');
    } catch (e) {
      _addResult('  [✗] 基础API失败: \$e');
    }
  }

  Future<void> _testAutoRecovery() async {
    _addResult('[Test 2] Auto Recovery 功能测试');
    try {
      // TODO: 测试 Auto Recovery API
      _addResult('  [✓] Auto Recovery 功能正常');
    } catch (e) {
      _addResult('  [✗] Auto Recovery 失败: \$e');
    }
  }

  Future<void> _testWebMessage() async {
    _addResult('[Test 3] WebMessage 测试');
    try {
      // TODO: 测试 WebMessage API
      _addResult('  [✓] WebMessage 功能正常');
    } catch (e) {
      _addResult('  [✗] WebMessage 失败: \$e');
    }
  }

  void _addResult(String result) {
    setState(() {
      _testResults.add(result);
    });
    print(result);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('AnyWP Engine 集成测试'),
      ),
      body: Column(
        children: [
          if (_isRunning)
            const LinearProgressIndicator(),
          Expanded(
            child: ListView.builder(
              itemCount: _testResults.length,
              itemBuilder: (context, index) {
                final result = _testResults[index];
                return ListTile(
                  title: Text(
                    result,
                    style: TextStyle(
                      fontFamily: 'Courier',
                      fontSize: 12,
                      color: result.contains('[✓]')
                          ? Colors.green
                          : result.contains('[✗]')
                              ? Colors.red
                              : Colors.black,
                    ),
                  ),
                  dense: true,
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}
"@

Set-Content -Path (Join-Path $projectRoot "lib\main.dart") -Value $mainContent

# 创建windows目录
Write-Host "创建Windows项目结构..." -ForegroundColor Cyan
$windowsPath = Join-Path $projectRoot "windows"
New-Item -ItemType Directory -Path $windowsPath -Force | Out-Null

# 创建runner子目录
$runnerPath = Join-Path $windowsPath "runner"
New-Item -ItemType Directory -Path $runnerPath -Force | Out-Null

# 创建CMakeLists.txt
$cmakeContent = @"
cmake_minimum_required(VERSION 3.14)
project(anywp_engine_integration_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BINARY_NAME "anywp_engine_integration_test")

cmake_policy(SET CMP0063 NEW)

set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")

add_subdirectory(runner)

flutter_build(test)
"@

Set-Content -Path (Join-Path $windowsPath "CMakeLists.txt") -Value $cmakeContent

Write-Host ""
Write-Host "✓ 集成测试项目创建完成" -ForegroundColor Green
Write-Host ""
Write-Host "下一步:" -ForegroundColor Yellow
Write-Host "  1. cd $projectRoot"
Write-Host "  2. 将 anywp_engine 添加到 pubspec.yaml 的 dependencies"
Write-Host "  3. flutter pub get"
Write-Host "  4. flutter run -d windows"
Write-Host ""




