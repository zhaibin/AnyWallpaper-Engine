#include "custom_scheme_handler.h"
#include "../utils/mime_type_detector.h"
#include "../utils/logger.h"
#include <shlobj.h>
#include <sstream>
#include <wrl/client.h>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace anywp_engine {

EventRegistrationToken CustomSchemeHandler::s_requestedToken_ = {};

HRESULT CustomSchemeHandler::Initialize(ICoreWebView2* webview) {
  if (!webview) {
    Logger::Instance().Error("CustomSchemeHandler", "Initialize - WebView is null");
    return E_INVALIDARG;
  }
  
  try {
    Logger::Instance().Info("CustomSchemeHandler", "Initialize - Registering anywp:// protocol");
    
    // 注册 anywp:// 协议过滤器
    HRESULT hr = webview->AddWebResourceRequestedFilter(
      L"anywp://*",
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL
    );
    
    if (FAILED(hr)) {
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to add web resource filter. HRESULT: " + std::to_string(hr));
      return hr;
    }
    
    // 注册请求处理事件
    hr = webview->add_WebResourceRequested(
      Callback<ICoreWebView2WebResourceRequestedEventHandler>(
        [](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
          return HandleRequest(sender, args);
        }
      ).Get(),
      &s_requestedToken_
    );
    
    if (FAILED(hr)) {
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to register WebResourceRequested handler. HRESULT: " + std::to_string(hr));
      return hr;
    }
    
    Logger::Instance().Info("CustomSchemeHandler", "Successfully registered anywp:// protocol");
    return S_OK;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CustomSchemeHandler", 
      std::string("Exception in Initialize: ") + e.what());
    return E_FAIL;
  }
}

void CustomSchemeHandler::Uninitialize() {
  Logger::Instance().Info("CustomSchemeHandler", "Uninitialize - Cleaning up");
  s_requestedToken_ = {};
}

HRESULT CustomSchemeHandler::HandleRequest(
  ICoreWebView2* webview,
  ICoreWebView2WebResourceRequestedEventArgs* args) {
  
  if (!webview || !args) {
    return E_INVALIDARG;
  }
  
  try {
    // 1. 获取请求 URL
    ComPtr<ICoreWebView2WebResourceRequest> request;
    args->get_Request(&request);
    
    LPWSTR uri_raw = nullptr;
    request->get_Uri(&uri_raw);
    std::wstring url(uri_raw);
    CoTaskMemFree(uri_raw);
    
    Logger::Instance().Debug("CustomSchemeHandler", 
      "Processing request: " + std::string(url.begin(), url.end()));
    
    // 2. 提取文件路径
    std::wstring encryptedPath;
    if (!ExtractFilePath(url, encryptedPath)) {
      Logger::Instance().Warn("CustomSchemeHandler", 
        "Invalid URL format: " + std::string(url.begin(), url.end()));
      
      ComPtr<ICoreWebView2Environment> env;
      ComPtr<ICoreWebView2_2> webview2;
      if (SUCCEEDED(webview->QueryInterface(IID_PPV_ARGS(&webview2)))) {
        webview2->get_Environment(&env);
      }
      return CreateErrorResponse(env.Get(), args, 400, L"Invalid URL format: expected anywp://file?path=...");
    }
    
    // 3. 验证路径安全性
    if (!ValidatePathSecurity(encryptedPath)) {
      Logger::Instance().Error("CustomSchemeHandler", 
        "Path security validation failed: " + std::string(encryptedPath.begin(), encryptedPath.end()));
      
      ComPtr<ICoreWebView2Environment> env;
      ComPtr<ICoreWebView2_2> webview2;
      if (SUCCEEDED(webview->QueryInterface(IID_PPV_ARGS(&webview2)))) {
        webview2->get_Environment(&env);
      }
      return CreateErrorResponse(env.Get(), args, 403, L"Forbidden: Invalid file path");
    }
    
    Logger::Instance().Debug("CustomSchemeHandler", 
      "Decrypting file: " + std::string(encryptedPath.begin(), encryptedPath.end()));
    
    // 4. 检测 MIME Type
    std::wstring mimeType = MimeTypeDetector::DetectFromFile(encryptedPath);
    if (mimeType.empty()) {
      mimeType = L"application/octet-stream";
    }
    
    // 5. 解密文件到内存流
    ComPtr<IStream> memStream;
    HRESULT hr = DecryptToStream(encryptedPath, memStream.GetAddressOf());
    
    if (FAILED(hr)) {
      Logger::Instance().Error("CustomSchemeHandler", 
        "Decryption failed for: " + std::string(encryptedPath.begin(), encryptedPath.end()) + 
        " HRESULT: " + std::to_string(hr));
      
      ComPtr<ICoreWebView2Environment> env;
      ComPtr<ICoreWebView2_2> webview2;
      if (SUCCEEDED(webview->QueryInterface(IID_PPV_ARGS(&webview2)))) {
        webview2->get_Environment(&env);
      }
      
      // 文件不存在返回 404，解密失败返回 500
      int statusCode = (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ? 404 : 500;
      return CreateErrorResponse(env.Get(), args, statusCode, 
        statusCode == 404 ? L"File not found" : L"Decryption failed");
    }
    
    // 6. 创建响应头
    std::wstring headers = L"Content-Type: " + mimeType + L"\r\n"
                          L"Cache-Control: max-age=31536000\r\n"
                          L"Access-Control-Allow-Origin: *";
    
    // 7. 创建 WebView2 响应
    ComPtr<ICoreWebView2Environment> env;
    ComPtr<ICoreWebView2_2> webview2;
    if (FAILED(webview->QueryInterface(IID_PPV_ARGS(&webview2)))) {
      Logger::Instance().Error("CustomSchemeHandler", "Failed to QueryInterface ICoreWebView2_2");
      return E_FAIL;
    }
    webview2->get_Environment(&env);
    
    ComPtr<ICoreWebView2WebResourceResponse> response;
    hr = env->CreateWebResourceResponse(
      memStream.Get(),
      200,
      L"OK",
      headers.c_str(),
      &response
    );
    
    if (FAILED(hr)) {
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to create WebResourceResponse. HRESULT: " + std::to_string(hr));
      return hr;
    }
    
    // 8. 设置响应
    args->put_Response(response.Get());
    
    Logger::Instance().Debug("CustomSchemeHandler", 
      "Successfully handled request - MimeType: " + std::string(mimeType.begin(), mimeType.end()));
    
    return S_OK;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CustomSchemeHandler", 
      std::string("Exception in HandleRequest: ") + e.what());
    return E_FAIL;
  }
}

// ============================================================================
// 路径提取和验证
// ============================================================================

bool CustomSchemeHandler::ExtractFilePath(const std::wstring& url, std::wstring& filePath) {
  // 验证协议前缀 "anywp://file?path="
  const std::wstring prefix = L"anywp://file?path=";
  if (url.length() < prefix.length() || url.substr(0, prefix.length()) != prefix) {
    return false;
  }
  
  // 提取路径部分
  filePath = url.substr(prefix.length());
  
  // URL 解码（处理 %20 等转义字符）
  std::wstring decoded;
  for (size_t i = 0; i < filePath.length(); i++) {
    if (filePath[i] == L'%' && i + 2 < filePath.length()) {
      // 解码 %XX
      wchar_t hex[3] = { filePath[i + 1], filePath[i + 2], 0 };
      wchar_t* end;
      int value = wcstol(hex, &end, 16);
      if (end == hex + 2) {
        decoded += static_cast<wchar_t>(value);
        i += 2;
        continue;
      }
    }
    decoded += filePath[i];
  }
  
  filePath = decoded;
  
  // 基本验证
  if (filePath.empty() || filePath.length() < 3) {
    return false;
  }
  
  return true;
}

bool CustomSchemeHandler::ValidatePathSecurity(const std::wstring& filePath) {
  // 1. 检查路径遍历攻击模式
  if (filePath.find(L"..") != std::wstring::npos) {
    return false;
  }
  
  // 2. 检查非法字符
  const std::wstring illegalChars = L"<>|\"";
  for (wchar_t ch : illegalChars) {
    if (filePath.find(ch) != std::wstring::npos) {
      return false;
    }
  }
  
  // 3. 验证是否为绝对路径（Windows: C:\ 或 \\）
  bool isAbsolute = false;
  if (filePath.length() >= 3 && filePath[1] == L':' && 
      (filePath[2] == L'\\' || filePath[2] == L'/')) {
    isAbsolute = true; // C:\path or C:/path
  } else if (filePath.length() >= 2 && filePath[0] == L'\\' && filePath[1] == L'\\') {
    isAbsolute = true; // UNC path \\server\share
  }
  
  if (!isAbsolute) {
    return false; // 必须使用绝对路径
  }
  
  return true;
}

// ============================================================================
// 加密/解密实现
// ============================================================================

HRESULT CustomSchemeHandler::DecryptToStream(
  const std::wstring& filePath,
  IStream** ppStream) {
  
  if (!ppStream) {
    return E_POINTER;
  }
  
  *ppStream = nullptr;
  
  try {
    // 1. 打开加密文件
    HANDLE hFile = CreateFileW(
      filePath.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
      DWORD error = GetLastError();
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to open file: " + std::string(filePath.begin(), filePath.end()) + 
        " Error: " + std::to_string(error));
      return HRESULT_FROM_WIN32(error);
    }
    
    // 2. 创建内存流
    ComPtr<IStream> memStream;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, memStream.GetAddressOf());
    if (FAILED(hr)) {
      CloseHandle(hFile);
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to create memory stream. HRESULT: " + std::to_string(hr));
      return hr;
    }
    
    // 3. 流式解密并写入内存流
    BYTE buffer[CHUNK_SIZE];
    DWORD bytesRead;
    bool isFirstChunk = true;
    
    while (ReadFile(hFile, buffer, CHUNK_SIZE, &bytesRead, NULL) && bytesRead > 0) {
      // 解密第一块（前64字节）
      if (isFirstChunk) {
        DeobfuscateBytes(buffer, bytesRead);
        isFirstChunk = false;
      }
      // 后续块不需要处理
      
      // 写入流
      ULONG written;
      hr = memStream.Get()->Write(buffer, bytesRead, &written);
      if (FAILED(hr) || written != bytesRead) {
        CloseHandle(hFile);
        Logger::Instance().Error("CustomSchemeHandler", 
          "Failed to write to memory stream. HRESULT: " + std::to_string(hr));
        return hr;
      }
    }
    
    CloseHandle(hFile);
    
    // 4. 重置流指针到开始位置
    LARGE_INTEGER zero = {};
    hr = memStream.Get()->Seek(zero, STREAM_SEEK_SET, NULL);
    if (FAILED(hr)) {
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to reset stream position. HRESULT: " + std::to_string(hr));
      return hr;
    }
    
    // 5. 返回流
    *ppStream = memStream.Detach();
    return S_OK;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CustomSchemeHandler", 
      std::string("Exception in DecryptToStream: ") + e.what());
    return E_FAIL;
  }
}

HRESULT CustomSchemeHandler::EncryptFile(
  const std::wstring& sourcePath,
  const std::wstring& destPath) {
  
  try {
    // 1. 打开源文件
    HANDLE hSource = CreateFileW(
      sourcePath.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL
    );
    
    if (hSource == INVALID_HANDLE_VALUE) {
      DWORD error = GetLastError();
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to open source file: " + std::string(sourcePath.begin(), sourcePath.end()) + 
        " Error: " + std::to_string(error));
      return HRESULT_FROM_WIN32(error);
    }
    
    // 2. 创建目标文件
    HANDLE hDest = CreateFileW(
      destPath.c_str(),
      GENERIC_WRITE,
      0,
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
    );
    
    if (hDest == INVALID_HANDLE_VALUE) {
      DWORD error = GetLastError();
      CloseHandle(hSource);
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to create dest file: " + std::string(destPath.begin(), destPath.end()) + 
        " Error: " + std::to_string(error));
      return HRESULT_FROM_WIN32(error);
    }
    
    // 3. 流式加密并写入目标文件
    BYTE buffer[CHUNK_SIZE];
    DWORD bytesRead, bytesWritten;
    bool isFirstChunk = true;
    
    while (ReadFile(hSource, buffer, CHUNK_SIZE, &bytesRead, NULL) && bytesRead > 0) {
      // 加密第一块（前64字节）
      if (isFirstChunk) {
        ObfuscateBytes(buffer, bytesRead);
        isFirstChunk = false;
      }
      // 后续块不需要处理
      
      // 写入文件
      if (!WriteFile(hDest, buffer, bytesRead, &bytesWritten, NULL) || bytesWritten != bytesRead) {
        DWORD error = GetLastError();
        CloseHandle(hSource);
        CloseHandle(hDest);
        Logger::Instance().Error("CustomSchemeHandler", 
          "Failed to write to dest file. Error: " + std::to_string(error));
        return HRESULT_FROM_WIN32(error);
      }
    }
    
    CloseHandle(hSource);
    CloseHandle(hDest);
    
    Logger::Instance().Info("CustomSchemeHandler", 
      "Successfully encrypted file: " + std::string(sourcePath.begin(), sourcePath.end()) + 
      " -> " + std::string(destPath.begin(), destPath.end()));
    
    return S_OK;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CustomSchemeHandler", 
      std::string("Exception in EncryptFile: ") + e.what());
    return E_FAIL;
  }
}

HRESULT CustomSchemeHandler::DecryptFile(
  const std::wstring& encryptedPath,
  const std::wstring& destPath) {
  
  // 解密逻辑与加密相同（XOR 对称）
  return EncryptFile(encryptedPath, destPath);
}

void CustomSchemeHandler::DeobfuscateBytes(BYTE* data, DWORD length) {
  // 只处理前64字节（或更少）
  DWORD count = min(length, OBFUSCATION_BYTE_COUNT);
  
  for (DWORD i = 0; i < count; i++) {
    data[i] ^= XOR_KEY;
  }
}

void CustomSchemeHandler::ObfuscateBytes(BYTE* data, DWORD length) {
  // XOR 是对称操作，加密和解密使用同一函数
  DeobfuscateBytes(data, length);
}

// ============================================================================
// 错误响应
// ============================================================================

HRESULT CustomSchemeHandler::CreateErrorResponse(
  ICoreWebView2Environment* env,
  ICoreWebView2WebResourceRequestedEventArgs* args,
  int statusCode,
  const std::wstring& message) {
  
  if (!env || !args) {
    return E_INVALIDARG;
  }
  
  try {
    // 创建空流
    ComPtr<IStream> emptyStream;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, emptyStream.GetAddressOf());
    if (FAILED(hr)) {
      return hr;
    }
    
    // 写入错误消息到流
    std::string errorMsg = "Error: ";
    errorMsg += std::string(message.begin(), message.end());
    ULONG written;
    emptyStream.Get()->Write(errorMsg.c_str(), static_cast<ULONG>(errorMsg.size()), &written);
    
    // 重置流指针
    LARGE_INTEGER zero = {};
    emptyStream.Get()->Seek(zero, STREAM_SEEK_SET, NULL);
    
    // 确定状态文本
    const wchar_t* reasonPhrase = L"Error";
    if (statusCode == 403) {
      reasonPhrase = L"Forbidden";
    } else if (statusCode == 404) {
      reasonPhrase = L"Not Found";
    } else if (statusCode == 400) {
      reasonPhrase = L"Bad Request";
    } else if (statusCode == 500) {
      reasonPhrase = L"Internal Server Error";
    }
    
    // 创建错误响应
    ComPtr<ICoreWebView2WebResourceResponse> response;
    hr = env->CreateWebResourceResponse(
      emptyStream.Get(),
      statusCode,
      reasonPhrase,
      L"Content-Type: text/plain; charset=utf-8",
      &response
    );
    
    if (FAILED(hr)) {
      return hr;
    }
    
    args->put_Response(response.Get());
    
    Logger::Instance().Warn("CustomSchemeHandler", 
      "Error response sent - Code: " + std::to_string(statusCode) + 
      " Message: " + std::string(message.begin(), message.end()));
    
    return S_OK;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CustomSchemeHandler", 
      std::string("Exception in CreateErrorResponse: ") + e.what());
    return E_FAIL;
  }
}

}  // namespace anywp_engine
