#!/bin/bash
# Quick test for macOS file server

cd /Users/zhaibin/Dev/anywp-engine

echo "========================================="
echo " Quick Test - macOS LocalFileServer"
echo "========================================="
echo ""

# Check if examples directory exists
if [ -d "examples" ]; then
    echo "✅ examples directory found"
    echo "   Path: $(pwd)/examples"
    echo ""
    echo "📁 Available test files:"
    ls -lh examples/*.html | awk '{print "   - " $9 " (" $5 ")"}'
    echo ""
else
    echo "❌ examples directory NOT found"
    exit 1
fi

# Test file path
TEST_FILE="examples/test_carousel_control.html"
if [ -f "$TEST_FILE" ]; then
    echo "✅ Test file exists: $TEST_FILE"
    echo "   Size: $(wc -c < $TEST_FILE) bytes"
else
    echo "❌ Test file NOT found: $TEST_FILE"
    exit 1
fi

echo ""
echo "========================================="
echo " Usage Instructions"
echo "========================================="
echo ""
echo "方式 1: 使用 Dart HTTP 服务器（推荐）"
echo "----------------------------------------"
echo "示例应用会自动启动 HTTP 服务器"
echo "URL: http://127.0.0.1:<随机端口>/examples/test_carousel_control.html"
echo ""
echo "方式 2: 使用 macOS LocalFileServer（新功能）"
echo "----------------------------------------"
echo "API 调用:"
echo "  final result = await AnyWPEngine.startFileServer("
echo "    rootPath: '$(pwd)',"
echo "  );"
echo "  // URL: localfile:///examples/test_carousel_control.html"
echo ""
echo "方式 3: 使用 file:// 协议（可能有 CORS 问题）"
echo "----------------------------------------"
echo "URL: file://$(pwd)/examples/test_carousel_control.html"
echo ""

echo "========================================="
echo " Troubleshooting"
echo "========================================="
echo ""
echo "如果 HTTP 服务器显示 404:"
echo "1. 检查示例应用日志中的 HTTP 服务器启动信息"
echo "2. 确认服务器找到了正确的项目根目录"
echo "3. 尝试使用 localfile:// 协议替代"
echo ""

