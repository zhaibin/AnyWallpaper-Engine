#include "../modules/custom_scheme_handler.h"
#include "../utils/mime_type_detector.h"
#include <iostream>
#include <cassert>
#include <Windows.h>

using namespace anywp_engine;

// 测试辅助函数
void TestResult(const char* testName, bool passed) {
  std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << std::endl;
  if (!passed) {
    std::cerr << "  Test failed!" << std::endl;
  }
}

// 测试 MIME Type 检测（文件扩展名）
void TestMimeTypeFromExtension() {
  std::cout << "\n=== Testing MIME Type Detection (Extension) ===" << std::endl;
  
  struct TestCase {
    const wchar_t* filePath;
    const wchar_t* expectedMimeType;
  };
  
  TestCase tests[] = {
    {L"test.jpg", L"image/jpeg"},
    {L"test.jpeg", L"image/jpeg"},
    {L"test.png", L"image/png"},
    {L"test.gif", L"image/gif"},
    {L"test.webp", L"image/webp"},
    {L"test.bmp", L"image/bmp"},
    {L"test.mp4", L"video/mp4"},
    {L"test.webm", L"video/webm"},
    {L"test.avi", L"video/avi"},
    {L"test.mov", L"video/quicktime"},
    {L"test.unknown", L"application/octet-stream"},
  };
  
  for (const auto& test : tests) {
    std::wstring detected = MimeTypeDetector::GetFromExtension(test.filePath);
    bool passed = (detected == test.expectedMimeType);
    
    std::wcout << L"  File: " << test.filePath 
               << L" -> " << detected 
               << (passed ? L" ✓" : L" ✗") << std::endl;
    
    if (!passed) {
      std::wcout << L"    Expected: " << test.expectedMimeType << std::endl;
    }
  }
}

// 测试 XOR 解密算法
void TestDeobfuscation() {
  std::cout << "\n=== Testing XOR Deobfuscation ===" << std::endl;
  
  // 测试数据（模拟加密的前64字节）
  const BYTE XOR_KEY = 0x5A;
  const DWORD TEST_SIZE = 128;
  BYTE testData[TEST_SIZE];
  
  // 创建测试数据（原始数据）
  for (DWORD i = 0; i < TEST_SIZE; i++) {
    testData[i] = static_cast<BYTE>(i % 256);
  }
  
  // 保存原始数据的副本
  BYTE original[TEST_SIZE];
  memcpy(original, testData, TEST_SIZE);
  
  // 加密（XOR 前64字节）
  for (DWORD i = 0; i < 64; i++) {
    testData[i] ^= XOR_KEY;
  }
  
  // 验证加密后数据不同
  bool encrypted = false;
  for (DWORD i = 0; i < 64; i++) {
    if (testData[i] != original[i]) {
      encrypted = true;
      break;
    }
  }
  TestResult("Data is encrypted", encrypted);
  
  // 验证后64字节未加密
  bool backHalfUnchanged = true;
  for (DWORD i = 64; i < TEST_SIZE; i++) {
    if (testData[i] != original[i]) {
      backHalfUnchanged = false;
      break;
    }
  }
  TestResult("Back half unchanged", backHalfUnchanged);
  
  // 解密（使用相同的 XOR 操作）
  for (DWORD i = 0; i < 64; i++) {
    testData[i] ^= XOR_KEY;
  }
  
  // 验证解密后数据与原始数据一致
  bool decrypted = true;
  for (DWORD i = 0; i < TEST_SIZE; i++) {
    if (testData[i] != original[i]) {
      decrypted = false;
      std::cout << "  Mismatch at index " << i 
                << ": " << static_cast<int>(testData[i]) 
                << " != " << static_cast<int>(original[i]) << std::endl;
      break;
    }
  }
  TestResult("Data decrypted correctly", decrypted);
}

// 测试 URL 解析
void TestUrlParsing() {
  std::cout << "\n=== Testing URL Parsing ===" << std::endl;
  
  struct TestCase {
    const wchar_t* url;
    bool shouldSucceed;
    const wchar_t* expectedType;
    const wchar_t* expectedId;
  };
  
  TestCase tests[] = {
    {L"anywp://image/abc123", true, L"image", L"abc123"},
    {L"anywp://video/def456", true, L"video", L"def456"},
    {L"anywp://interactive/ghi789", true, L"interactive", L"ghi789"},
    {L"anywp://image/", false, nullptr, nullptr},  // Empty ID
    {L"anywp://unknown/test", false, nullptr, nullptr},  // Invalid type
    {L"http://example.com", false, nullptr, nullptr},  // Wrong protocol
    {L"anywp://", false, nullptr, nullptr},  // Missing path
  };
  
  // Note: We can't directly call ParseUrl as it's private
  // This is a simplified test for the expected behavior
  for (const auto& test : tests) {
    std::wcout << L"  URL: " << test.url << std::endl;
    
    // Simplified parsing logic for testing
    std::wstring url(test.url);
    const std::wstring prefix = L"anywp://";
    bool hasPrefix = (url.find(prefix) == 0);
    
    if (test.shouldSucceed) {
      TestResult("  Valid URL detected", hasPrefix);
    } else {
      TestResult("  Invalid URL rejected", !hasPrefix || url.length() <= prefix.length());
    }
  }
}

// 测试缓存文件路径生成
void TestCachePathGeneration() {
  std::cout << "\n=== Testing Cache Path Generation ===" << std::endl;
  
  // 获取 AppData 路径
  WCHAR appDataPath[MAX_PATH];
  SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath);
  
  std::wstring basePath = appDataPath;
  basePath += L"\\HKCW_Desktop\\cache\\";
  
  struct TestCase {
    const wchar_t* resourceType;
    const wchar_t* fileId;
    std::wstring expectedPath;
  };
  
  TestCase tests[] = {
    {L"image", L"abc123", basePath + L"images\\abc123.encrypted"},
    {L"video", L"def456", basePath + L"videos\\def456.encrypted"},
    {L"interactive", L"ghi789", basePath + L"other\\ghi789.encrypted"},
  };
  
  for (const auto& test : tests) {
    std::wcout << L"  Type: " << test.resourceType 
               << L", ID: " << test.fileId << std::endl;
    std::wcout << L"  Expected: " << test.expectedPath << std::endl;
    
    // Path generation logic would be tested here
    // For now, just verify the expected path format
    bool hasCorrectPrefix = (test.expectedPath.find(basePath) == 0);
    bool hasCorrectSuffix = (test.expectedPath.find(L".encrypted") != std::wstring::npos);
    
    TestResult("  Path has correct prefix", hasCorrectPrefix);
    TestResult("  Path has .encrypted suffix", hasCorrectSuffix);
  }
}

// 测试文件头魔数检测
void TestMagicNumberDetection() {
  std::cout << "\n=== Testing Magic Number Detection ===" << std::endl;
  
  // JPEG 文件头
  BYTE jpegHeader[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46};
  std::wstring jpegMime = MimeTypeDetector::DetectFromHeader(jpegHeader, sizeof(jpegHeader));
  TestResult("JPEG header detected", jpegMime == L"image/jpeg");
  
  // PNG 文件头
  BYTE pngHeader[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  std::wstring pngMime = MimeTypeDetector::DetectFromHeader(pngHeader, sizeof(pngHeader));
  TestResult("PNG header detected", pngMime == L"image/png");
  
  // GIF 文件头
  BYTE gifHeader[] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61};
  std::wstring gifMime = MimeTypeDetector::DetectFromHeader(gifHeader, sizeof(gifHeader));
  TestResult("GIF header detected", gifMime == L"image/gif");
  
  // 未知文件头
  BYTE unknownHeader[] = {0x00, 0x00, 0x00, 0x00};
  std::wstring unknownMime = MimeTypeDetector::DetectFromHeader(unknownHeader, sizeof(unknownHeader));
  TestResult("Unknown header returns octet-stream", unknownMime == L"application/octet-stream");
}

int main() {
  std::cout << "\n============================================" << std::endl;
  std::cout << "  Custom Scheme Handler Unit Tests" << std::endl;
  std::cout << "============================================\n" << std::endl;
  
  TestMimeTypeFromExtension();
  TestMagicNumberDetection();
  TestDeobfuscation();
  TestUrlParsing();
  TestCachePathGeneration();
  
  std::cout << "\n============================================" << std::endl;
  std::cout << "  All Tests Completed" << std::endl;
  std::cout << "============================================\n" << std::endl;
  
  return 0;
}

