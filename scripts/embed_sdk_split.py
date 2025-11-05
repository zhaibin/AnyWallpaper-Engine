#!/usr/bin/env python3
"""
将 anywp_sdk.js 分段嵌入到 C++ 代码中（避免字符串长度限制）
"""

import os

def main():
    # 读取 SDK 文件
    sdk_path = os.path.join(os.path.dirname(__file__), '..', 'windows', 'anywp_sdk.js')
    
    with open(sdk_path, 'r', encoding='utf-8') as f:
        sdk_content = f.read()
    
    # 移除 BOM
    if sdk_content.startswith('\ufeff'):
        sdk_content = sdk_content[1:]
    
    # 分段（每段 10000 字符）
    chunk_size = 10000
    chunks = []
    for i in range(0, len(sdk_content), chunk_size):
        chunks.append(sdk_content[i:i+chunk_size])
    
    # 生成 C++ 代码
    cpp_code = '''// Auto-generated embedded SDK - DO NOT EDIT
// Generated from windows/anywp_sdk.js
// SDK v4.2.0 with drag support

#ifndef EMBEDDED_SDK_H_
#define EMBEDDED_SDK_H_

#include <string>

namespace anywp_engine {

inline std::string GetEmbeddedSDK() {
  std::string sdk;
'''
    
    for i, chunk in enumerate(chunks):
        # 不转义，直接使用原始字符串字面量
        # R"(...)" 已经支持所有字符包括换行
        cpp_code += f'  sdk += R"SDK_CHUNK(\n{chunk}\n)SDK_CHUNK";\n'
    
    cpp_code += '''
  return sdk;
}

}  // namespace anywp_engine

#endif  // EMBEDDED_SDK_H_
'''
    
    # 输出
    output_path = os.path.join(os.path.dirname(__file__), '..', 'windows', 'embedded_sdk.h')
    with open(output_path, 'w', encoding='utf-8-sig') as f:  # 使用 UTF-8 with BOM
        f.write(cpp_code)
    
    print(f'[OK] SDK embedded to: {output_path}')
    print(f'[INFO] SDK size: {len(sdk_content)} characters')
    print(f'[INFO] Split into {len(chunks)} chunks')

if __name__ == '__main__':
    main()

