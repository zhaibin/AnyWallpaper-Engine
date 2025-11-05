#!/usr/bin/env python3
"""
将 anywp_sdk.js 嵌入到 C++ 代码中
"""

import os
import sys

def main():
    # 读取 SDK 文件
    sdk_path = os.path.join(os.path.dirname(__file__), '..', 'windows', 'anywp_sdk.js')
    
    with open(sdk_path, 'r', encoding='utf-8') as f:
        sdk_content = f.read()
    
    # 转义特殊字符
    sdk_content = sdk_content.replace('\\', '\\\\')
    sdk_content = sdk_content.replace('"', '\\"')
    sdk_content = sdk_content.replace('\r\n', '\\n')
    sdk_content = sdk_content.replace('\n', '\\n')
    
    # 生成 C++ 代码
    cpp_code = f'''// Auto-generated embedded SDK - DO NOT EDIT
// Generated from windows/anywp_sdk.js

const char* EMBEDDED_SDK_V4_2 = R"SDK_CONTENT(
{sdk_content.replace(chr(92) + 'n', chr(10))}
)SDK_CONTENT";
'''
    
    # 输出
    output_path = os.path.join(os.path.dirname(__file__), '..', 'windows', 'embedded_sdk.h')
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(cpp_code)
    
    print(f'[OK] SDK embedded to: {output_path}')
    print(f'[INFO] SDK size: {len(sdk_content)} characters')

if __name__ == '__main__':
    main()

