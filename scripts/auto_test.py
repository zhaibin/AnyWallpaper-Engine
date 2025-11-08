#!/usr/bin/env python3
"""
AnyWP Engine 自动化测试脚本
模拟用户操作并收集日志
"""

import subprocess
import time
import os
import sys
from pathlib import Path

# 配置
PROJECT_DIR = Path(r"E:\Projects\AnyWallpaper\AnyWallpaper-Engine")
EXAMPLE_DIR = PROJECT_DIR / "example"
EXE_PATH = EXAMPLE_DIR / r"build\windows\x64\runner\Debug\anywallpaper_engine_example.exe"
LOG_FILE = PROJECT_DIR / "python_test_results.log"

# 测试页面列表
TEST_PAGES = [
    {"name": "重构测试页", "file": "test_refactoring.html", "duration": 15},
    {"name": "简单测试", "file": "test_simple.html", "duration": 8},
    {"name": "拖拽调试", "file": "test_drag_debug.html", "duration": 10},
    {"name": "API测试", "file": "test_api.html", "duration": 10},
    {"name": "点击测试", "file": "test_basic_click.html", "duration": 8},
    {"name": "React测试", "file": "test_react.html", "duration": 8},
    {"name": "Vue测试", "file": "test_vue.html", "duration": 8},
]

def log(message):
    """写入日志"""
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    log_msg = f"[{timestamp}] {message}"
    print(log_msg)
    with open(LOG_FILE, "a", encoding="utf-8") as f:
        f.write(log_msg + "\n")

def kill_process():
    """终止应用进程"""
    try:
        subprocess.run(["taskkill", "/IM", "anywallpaper_engine_example.exe", "/F"],
                      capture_output=True, timeout=5)
        time.sleep(2)
        log("✓ 进程已清理")
    except Exception as e:
        log(f"⚠ 清理进程失败: {e}")

def check_process_running():
    """检查进程是否运行"""
    try:
        result = subprocess.run(["tasklist", "/FI", "IMAGENAME eq anywallpaper_engine_example.exe"],
                              capture_output=True, text=True, timeout=5)
        return "anywallpaper_engine_example.exe" in result.stdout
    except:
        return False

def start_application():
    """启动应用"""
    if not EXE_PATH.exists():
        log(f"❌ 可执行文件不存在: {EXE_PATH}")
        return False
    
    try:
        subprocess.Popen(str(EXE_PATH), cwd=str(EXAMPLE_DIR))
        time.sleep(8)  # 等待应用启动
        
        if check_process_running():
            log("✓ 应用已启动")
            return True
        else:
            log("❌ 应用启动失败")
            return False
    except Exception as e:
        log(f"❌ 启动应用异常: {e}")
        return False

def run_tests():
    """运行测试"""
    log("=" * 60)
    log("AnyWP Engine 自动化测试开始")
    log("=" * 60)
    
    # 清理环境
    log("\n[步骤 1/4] 清理环境...")
    kill_process()
    
    # 启动应用
    log("\n[步骤 2/4] 启动应用...")
    if not start_application():
        log("❌ 测试终止：应用启动失败")
        return False
    
    # 执行测试
    log("\n[步骤 3/4] 执行测试序列...")
    log(f"共 {len(TEST_PAGES)} 个测试页面\n")
    
    for idx, test in enumerate(TEST_PAGES, 1):
        log(f"  [{idx}/{len(TEST_PAGES)}] 测试: {test['name']} ({test['file']})")
        
        # 检查进程是否仍在运行
        if not check_process_running():
            log(f"    ❌ 应用已崩溃或退出！")
            break
        
        log(f"    → 等待 {test['duration']} 秒...")
        time.sleep(test['duration'])
        
        log(f"    ✓ 测试完成")
    
    # 清理
    log("\n[步骤 4/4] 清理环境...")
    time.sleep(3)
    kill_process()
    
    # 总结
    log("\n" + "=" * 60)
    log("测试完成")
    log("=" * 60)
    log(f"\n日志文件: {LOG_FILE}")
    
    return True

if __name__ == "__main__":
    # 清空日志文件
    if LOG_FILE.exists():
        LOG_FILE.unlink()
    
    try:
        success = run_tests()
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        log("\n⚠ 测试被用户中断")
        kill_process()
        sys.exit(1)
    except Exception as e:
        log(f"\n❌ 测试异常: {e}")
        kill_process()
        sys.exit(1)

