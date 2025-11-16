#include "mime_type_detector.h"
#include <algorithm>
#include <cctype>

namespace anywp_engine {

// 魔数定义（支持常见图片和视频格式）
const std::vector<MimeTypeDetector::MagicNumber> MimeTypeDetector::s_magicNumbers = {
  // JPEG
  {
    {0xFF, 0xD8, 0xFF},
    {0xFF, 0xFF, 0xFF},
    0,
    L"image/jpeg"
  },
  
  // PNG
  {
    {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"image/png"
  },
  
  // GIF (GIF87a 或 GIF89a)
  {
    {0x47, 0x49, 0x46, 0x38},  // "GIF8"
    {0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"image/gif"
  },
  
  // WebP
  {
    {0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x45, 0x42, 0x50},  // "RIFF????WEBP"
    {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"image/webp"
  },
  
  // BMP
  {
    {0x42, 0x4D},  // "BM"
    {0xFF, 0xFF},
    0,
    L"image/bmp"
  },
  
  // ICO
  {
    {0x00, 0x00, 0x01, 0x00},
    {0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"image/x-icon"
  },
  
  // MP4 (ISO Base Media file format)
  {
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x74, 0x79, 0x70},  // "????ftyp"
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"video/mp4"
  },
  
  // WebM
  {
    {0x1A, 0x45, 0xDF, 0xA3},
    {0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"video/webm"
  },
  
  // AVI (RIFF AVI)
  {
    {0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x41, 0x56, 0x49, 0x20},  // "RIFF????AVI "
    {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"video/avi"
  },
  
  // MOV (QuickTime)
  {
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x74, 0x79, 0x70, 0x71, 0x74},  // "????ftypqt"
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    0,
    L"video/quicktime"
  },
};

// 扩展名映射（备用方案）
const std::map<std::wstring, std::wstring> MimeTypeDetector::s_extensionMap = {
  // Images
  {L".jpg", L"image/jpeg"},
  {L".jpeg", L"image/jpeg"},
  {L".png", L"image/png"},
  {L".gif", L"image/gif"},
  {L".webp", L"image/webp"},
  {L".bmp", L"image/bmp"},
  {L".ico", L"image/x-icon"},
  {L".svg", L"image/svg+xml"},
  
  // Videos
  {L".mp4", L"video/mp4"},
  {L".webm", L"video/webm"},
  {L".avi", L"video/avi"},
  {L".mov", L"video/quicktime"},
  {L".wmv", L"video/x-ms-wmv"},
  {L".flv", L"video/x-flv"},
  {L".mkv", L"video/x-matroska"},
  
  // HTML/Web
  {L".html", L"text/html"},
  {L".htm", L"text/html"},
  {L".css", L"text/css"},
  {L".js", L"application/javascript"},
  {L".json", L"application/json"},
};

std::wstring MimeTypeDetector::DetectFromFile(const std::wstring& filePath) {
  // 1. 打开文件
  HANDLE hFile = CreateFileW(
    filePath.c_str(),
    GENERIC_READ,
    FILE_SHARE_READ,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );
  
  if (hFile == INVALID_HANDLE_VALUE) {
    // 文件打开失败，尝试从扩展名推断
    return GetFromExtension(filePath);
  }
  
  // 2. 读取文件头（前64字节足够检测所有格式）
  const DWORD HEADER_SIZE = 64;
  BYTE header[HEADER_SIZE] = {0};
  DWORD bytesRead = 0;
  
  BOOL success = ReadFile(hFile, header, HEADER_SIZE, &bytesRead, NULL);
  CloseHandle(hFile);
  
  if (!success || bytesRead == 0) {
    // 读取失败，尝试从扩展名推断
    return GetFromExtension(filePath);
  }
  
  // 3. 从文件头检测
  std::wstring mimeType = DetectFromHeader(header, bytesRead);
  
  // 4. 如果检测失败，尝试从扩展名推断
  if (mimeType == L"application/octet-stream") {
    std::wstring extMime = GetFromExtension(filePath);
    if (extMime != L"application/octet-stream") {
      mimeType = extMime;
    }
  }
  
  return mimeType;
}

std::wstring MimeTypeDetector::DetectFromHeader(const BYTE* header, size_t size) {
  if (header == nullptr || size == 0) {
    return L"application/octet-stream";
  }
  
  // 遍历所有魔数模式
  for (const auto& magic : s_magicNumbers) {
    if (MatchPattern(header, size, magic)) {
      return magic.mimeType;
    }
  }
  
  return L"application/octet-stream";
}

std::wstring MimeTypeDetector::GetFromExtension(const std::wstring& filePath) {
  // 提取扩展名
  size_t dotPos = filePath.find_last_of(L'.');
  if (dotPos == std::wstring::npos) {
    return L"application/octet-stream";
  }
  
  std::wstring ext = filePath.substr(dotPos);
  
  // 转换为小写
  std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
  
  // 查找映射
  auto it = s_extensionMap.find(ext);
  if (it != s_extensionMap.end()) {
    return it->second;
  }
  
  return L"application/octet-stream";
}

bool MimeTypeDetector::MatchPattern(const BYTE* data, size_t dataSize, const MagicNumber& magic) {
  // 检查偏移量是否有效
  if (magic.offset >= dataSize) {
    return false;
  }
  
  // 检查剩余数据是否足够
  size_t remainingSize = dataSize - magic.offset;
  if (remainingSize < magic.pattern.size()) {
    return false;
  }
  
  // 应用掩码进行匹配
  const BYTE* checkData = data + magic.offset;
  for (size_t i = 0; i < magic.pattern.size(); i++) {
    BYTE maskedData = checkData[i] & magic.mask[i];
    BYTE maskedPattern = magic.pattern[i] & magic.mask[i];
    
    if (maskedData != maskedPattern) {
      return false;
    }
  }
  
  return true;
}

}  // namespace anywp_engine

