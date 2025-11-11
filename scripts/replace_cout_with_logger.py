#!/usr/bin/env python3
"""
Script to replace std::cout with Logger calls in C++ files
"""
import re
import sys
from pathlib import Path

def replace_cout_with_logger(content):
    """Replace std::cout patterns with Logger calls"""
    
    # Pattern 1: Simple INFO logs
    # std::cout << "[AnyWP] message" << std::endl;
    content = re.sub(
        r'std::cout\s*<<\s*"\[AnyWP\]\s+([^"]*?)"\s*<<\s*std::endl;',
        r'Logger::Instance().Info("Plugin", "\1");',
        content
    )
    
    # Pattern 2: ERROR logs
    # std::cout << "[AnyWP] ERROR: message" << std::endl;
    content = re.sub(
        r'std::cout\s*<<\s*"\[AnyWP\]\s+ERROR:\s+([^"]*?)"\s*<<\s*std::endl;',
        r'Logger::Instance().Error("Plugin", "\1");',
        content
    )
    
    # Pattern 3: WARNING logs
    # std::cout << "[AnyWP] WARNING: message" << std::endl;
    content = re.sub(
        r'std::cout\s*<<\s*"\[AnyWP\]\s+WARNING:\s+([^"]*?)"\s*<<\s*std::endl;',
        r'Logger::Instance().Warning("Plugin", "\1");',
        content
    )
    
    # Pattern 4: Refactor logs
    # std::cout << "[AnyWP] [Refactor] message" << std::endl;
    content = re.sub(
        r'std::cout\s*<<\s*"\[AnyWP\]\s+\[Refactor\]\s+([^"]*?)"\s*<<\s*std::endl;',
        r'Logger::Instance().Info("Refactor", "\1");',
        content
    )
    
    # Pattern 5: Logs with variables (simple concat)
    # std::cout << "[AnyWP] message " << var << std::endl;
    # This is more complex, handle manually or with more sophisticated regex
    
    return content

def process_file(file_path):
    """Process a single file"""
    try:
        print(f"Processing: {file_path}")
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        content = replace_cout_with_logger(content)
        
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ Modified {file_path}")
            return True
        else:
            print(f"  - No changes in {file_path}")
            return False
    except Exception as e:
        print(f"  ✗ Error processing {file_path}: {e}")
        return False

def main():
    if len(sys.argv) < 2:
        print("Usage: python replace_cout_with_logger.py <file1> [file2] ...")
        sys.exit(1)
    
    files = sys.argv[1:]
    modified_count = 0
    
    for file_path in files:
        if process_file(file_path):
            modified_count += 1
    
    print(f"\nTotal files modified: {modified_count}/{len(files)}")

if __name__ == "__main__":
    main()

