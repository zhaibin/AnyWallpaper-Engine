#pragma once
#include <string>
#include <vector>
#include <map>
#include <Windows.h>

namespace anywp_engine {

/// MIME Type 检测器
/// 
/// 根据文件头魔数（Magic Numbers）和文件扩展名检测文件的 MIME Type。
/// 支持常见的图片和视频格式。
class MimeTypeDetector {
public:
  /// 从文件路径检测 MIME Type
  /// 
  /// @param filePath 文件路径
  /// @return MIME Type 字符串（如 "image/jpeg"）
  static std::wstring DetectFromFile(const std::wstring& filePath);
  
  /// 从文件头字节检测 MIME Type
  /// 
  /// @param header 文件头字节数组
  /// @param size 字节数组大小
  /// @return MIME Type 字符串
  static std::wstring DetectFromHeader(const BYTE* header, size_t size);
  
  /// 从文件扩展名获取 MIME Type（备用方案）
  /// 
  /// @param filePath 文件路径
  /// @return MIME Type 字符串
  static std::wstring GetFromExtension(const std::wstring& filePath);
  
private:
  /// 魔数模式定义
  struct MagicNumber {
    std::vector<BYTE> pattern;    // 魔数模式
    std::vector<BYTE> mask;       // 掩码（0xFF = 必须匹配，0x00 = 忽略）
    size_t offset;                // 偏移量
    std::wstring mimeType;        // MIME Type
  };
  
  /// 预定义的魔数模式
  static const std::vector<MagicNumber> s_magicNumbers;
  
  /// 扩展名到 MIME Type 的映射
  static const std::map<std::wstring, std::wstring> s_extensionMap;
  
  /// 匹配魔数模式
  /// 
  /// @param data 待检测数据
  /// @param dataSize 数据大小
  /// @param magic 魔数模式
  /// @return 是否匹配
  static bool MatchPattern(const BYTE* data, size_t dataSize, const MagicNumber& magic);
};

}  // namespace anywp_engine

