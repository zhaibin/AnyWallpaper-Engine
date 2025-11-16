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
    
    Logger::Instance().Info("CustomSchemeHandler", "Initialize - Successfully registered anywp:// protocol");
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
      "HandleRequest - Processing request: " + std::string(url.begin(), url.end()));
    
    // 2. 解析 URL
    std::wstring resourceType, fileId;
    if (!ParseUrl(url, resourceType, fileId)) {
      Logger::Instance().Warn("CustomSchemeHandler", 
        "Invalid anywp:// URL format: " + std::string(url.begin(), url.end()));
      
      wil::com_ptr<ICoreWebView2Environment> env;
      webview->get_Environment(&env);
      return CreateErrorResponse(env.get(), args, 400, L"Invalid URL format");
    }
    
    // 3. 构建加密文件路径
    std::wstring encryptedPath = GetCacheFilePath(resourceType, fileId);
    
    Logger::Instance().Debug("CustomSchemeHandler", 
      "Decrypting cache file: " + std::string(encryptedPath.begin(), encryptedPath.end()));
    
    // 4. 检测 MIME Type
    std::wstring mimeType = MimeTypeDetector::DetectFromFile(encryptedPath);
    
    // 5. 解密文件到内存流
    ComPtr<IStream> memStream;
    HRESULT hr = DecryptToStream(encryptedPath, memStream.GetAddressOf());
    
    if (FAILED(hr)) {
      Logger::Instance().Error("CustomSchemeHandler", 
        "Failed to decrypt file: " + std::string(encryptedPath.begin(), encryptedPath.end()) + 
        " HRESULT: " + std::to_string(hr));
      
      ComPtr<ICoreWebView2Environment> env;
      webview->get_Environment(&env);
      
      // 文件不存在返回 404，其他错误返回 500
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
    webview->get_Environment(&env);
    
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
      "Successfully handled anywp:// request: " + std::string(url.begin(), url.end()) + 
      " MimeType: " + std::string(mimeType.begin(), mimeType.end()));
    
    return S_OK;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CustomSchemeHandler", 
      std::string("Exception in HandleRequest: ") + e.what());
    return E_FAIL;
  }
}

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
        "Failed to open encrypted file: " + std::string(filePath.begin(), filePath.end()) + 
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
          "Failed to write to memory stream. HRESULT: " + std::to_string(hr) + 
          " BytesRead: " + std::to_string(bytesRead) + 
          " BytesWritten: " + std::to_string(written));
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

void CustomSchemeHandler::DeobfuscateBytes(BYTE* data, DWORD length) {
  // 只处理前64字节（或更少）
  DWORD count = min(length, OBFUSCATION_BYTE_COUNT);
  
  for (DWORD i = 0; i < count; i++) {
    data[i] ^= XOR_KEY;
  }
}

std::wstring CustomSchemeHandler::GetCacheFilePath(
  const std::wstring& resourceType,
  const std::wstring& fileId) {
  
  // 获取 AppData 路径
  WCHAR appDataPath[MAX_PATH];
  SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath);
  
  // 构建缓存路径
  std::wstring cachePath = appDataPath;
  cachePath += L"\\AnyWP_Cache\\";
  
  // 添加资源类型子目录
  if (resourceType == L"image") {
    cachePath += L"images\\";
  } else if (resourceType == L"video") {
    cachePath += L"videos\\";
  } else {
    cachePath += L"other\\";
  }
  
  // 添加文件名
  cachePath += fileId;
  cachePath += L".encrypted";
  
  return cachePath;
}

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
    if (statusCode == 404) {
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
      L"Content-Type: text/plain",
      &response
    );
    
    if (FAILED(hr)) {
      return hr;
    }
    
    args->put_Response(response.Get());
    return S_OK;
    
  } catch (const std::exception& e) {
    Logger::Instance().Error("CustomSchemeHandler", 
      std::string("Exception in CreateErrorResponse: ") + e.what());
    return E_FAIL;
  }
}

bool CustomSchemeHandler::ParseUrl(
  const std::wstring& url,
  std::wstring& resourceType,
  std::wstring& fileId) {
  
  // anywp://image/abc123
  // anywp://video/def456
  
  const std::wstring prefix = L"anywp://";
  if (url.find(prefix) != 0) {
    return false;
  }
  
  // 移除协议前缀
  std::wstring path = url.substr(prefix.length());
  
  // 查找第一个斜杠
  size_t slashPos = path.find(L'/');
  if (slashPos == std::wstring::npos) {
    return false;
  }
  
  // 提取资源类型和文件ID
  resourceType = path.substr(0, slashPos);
  fileId = path.substr(slashPos + 1);
  
  // 验证资源类型
  if (resourceType != L"image" && resourceType != L"video" && resourceType != L"interactive") {
    return false;
  }
  
  // 验证文件ID非空
  if (fileId.empty()) {
    return false;
  }
  
  return true;
}

}  // namespace anywp_engine

