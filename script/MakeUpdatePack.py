#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MakeUpdatePack.py - Update Package Generator Script

Function:
1. Create "update" folder in target path
2. Copy "TZS100_VPU.bin" to target folder
"""

import os
import sys
import shutil
import struct
from pathlib import Path


def calculate_crc32_from_data(data):
    """计算数据的CRC32校验和，与C语言实现完全一致"""
    try:
        # 生成CRC32查找表（256字节模式，多项式0xEDB88320）
        def generate_crc32_table():
            table = [0] * 256
            for i in range(256):
                crc = i
                for j in range(8):
                    if crc & 1:
                        crc = (crc >> 1) ^ 0xEDB88320
                    else:
                        crc = crc >> 1
                table[i] = crc
            return table
        
        # 生成CRC32查找表
        crc32_table = generate_crc32_table()
        
        # 分块计算，与C语言实现保持一致（256字节/块）
        chunk_size = 256
        data_len = len(data)
        offset = 0
        
        # 处理第一个数据块（相当于C语言的第一次调用）
        if data_len > 0:
            first_chunk_size = min(chunk_size, data_len)
            first_chunk = data[offset:offset+first_chunk_size]
            
            # 第一次调用：使用TRUE参数（重置为初始值0xFFFFFFFF）
            crc_value = 0xFFFFFFFF
            
            # 使用查找表计算CRC（256字节模式）
            for byte in first_chunk:
                # C语言实现：Crc_Result = Crc32_Table256[((uint8)(Crc_Result & 0xFFU)) ^ *DataPtr] ^ (Crc_Result >> 8U)
                crc_value = crc32_table[(crc_value & 0xFF) ^ byte] ^ (crc_value >> 8)
            
            # 第一次调用后应用最终XOR
            crc_value ^= 0xFFFFFFFF
            
            offset += first_chunk_size
        else:
            # 空数据的情况
            crc_value = 0xFFFFFFFF ^ 0xFFFFFFFF  # 初始值XOR最终值
        
        # 处理后续数据块（相当于C语言的后续调用）
        while offset < data_len:
            chunk_size_current = min(chunk_size, data_len - offset)
            chunk = data[offset:offset+chunk_size_current]
            
            # 后续调用：使用FALSE参数（撤销之前的XOR，然后计算）
            crc_value ^= 0xFFFFFFFF  # 撤销之前的最终XOR
            
            # 使用查找表计算CRC
            for byte in chunk:
                crc_value = crc32_table[(crc_value & 0xFF) ^ byte] ^ (crc_value >> 8)
            
            # 应用最终XOR
            crc_value ^= 0xFFFFFFFF
            
            offset += chunk_size_current
        
        return crc_value & 0xFFFFFFFF
    except Exception as e:
        print(f"[ERROR] Failed to calculate CRC32: {e}")
        return 0

def calculate_crc32(file_path):
    """计算文件的CRC32校验和，与C语言实现保持一致"""
    try:
        with open(file_path, 'rb') as f:
            data = f.read()
        return calculate_crc32_from_data(data)
    except Exception as e:
        print(f"[ERROR] Failed to calculate CRC32: {e}")
        return 0


def append_crc32_to_file(file_path):
    """将CRC32校验和追加到文件末尾"""
    try:
        # 计算固件数据的CRC（不包括CRC值本身）
        with open(file_path, 'rb') as f:
            file_data = f.read()
        
        # 计算原始数据的CRC（不包括将要追加的4字节）
        crc_value = calculate_crc32_from_data(file_data)
        
        # 以二进制追加模式打开文件
        with open(file_path, 'ab') as f:
            # 将CRC32值以大端格式写入（4字节）
            f.write(struct.pack('>I', crc_value))
        
        #print(f"[INFO] CRC32 value (0x{crc_value:08X}) appended to file")
        return crc_value
        
    except Exception as e:
        print(f"[ERROR] Failed to append CRC32: {e}")
        return 0


def rename_file_with_size(original_path, new_name_base):
    """根据文件大小重命名文件"""
    try:
        # 获取文件大小
        file_size = os.path.getsize(original_path)
        
        # 构建新文件名
        file_dir = os.path.dirname(original_path)
        file_ext = os.path.splitext(original_path)[1]
        new_filename = f"{new_name_base}_{file_size}{file_ext}"
        new_path = os.path.join(file_dir, new_filename)
        
        # 重命名文件
        os.rename(original_path, new_path)
        
        #print(f"[INFO] File renamed: {os.path.basename(original_path)} -> {new_filename}")
        return new_path
        
    except Exception as e:
        print(f"[ERROR] Failed to rename file: {e}")
        return original_path


def get_app_version():
    """从version.c文件中获取完整的APP_VERSION字符串"""
    try:
        # 获取当前脚本所在目录
        script_dir = os.path.dirname(os.path.abspath(__file__))
        
        # version.c文件路径
        version_file = os.path.join(script_dir, '..', 'src', 'service', 'version', 'version.c')
        
        if not os.path.exists(version_file):
            print("[WARNING] version.c file not found, using default name")
            return "update"
        
        # 读取version.c文件内容
        with open(version_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 使用正则表达式查找各个版本组件
        import re
        
        # 查找APP版本的宏定义（跳过BOOT版本）
        components = {}
        
        # 分析文件结构，找到APP版本区域
        lines = content.split('\n')
        in_app_section = False
        
        for i, line in enumerate(lines):
            stripped_line = line.strip()
            
            # 检查是否进入APP版本区域（在#else之后）
            if stripped_line == '#else':
                in_app_section = True
                continue
            # 检查是否离开APP版本区域（在#endif之前）
            elif stripped_line.startswith('#endif') and in_app_section:
                break
            
            # 如果在APP版本区域，提取宏定义
            if in_app_section:
                patterns = {
                    'MOUDLE_TYPE': r'#define\s+MOUDLE_TYPE\s+"([^"]+)"',
                    'PROJECT_CODE': r'#define\s+PROJECT_CODE\s+"([^"]+)"'
                }
                
                for key, pattern in patterns.items():
                    match = re.search(pattern, stripped_line)
                    if match:
                        components[key] = match.group(1)
                        #print(f"[INFO] Found APP {key}: {components[key]}")
        
        # 查找全局的版本定义（不在条件编译内的）
        global_patterns = {
            'SOFTWARE_VER': r'#define\s+SOFTWARE_VER\s+"([^"]+)"',
            'DEBUG_VER': r'#define\s+DEBUG_VER\s+"([^"]+)"',
            'RELEASE_DATE': r'#define\s+RELEASE_DATE\s+"([^"]+)"',
            'HARDWARE_VER': r'#define\s+HARDWARE_VER\s+"([^"]+)"'
        }
        
        for key, pattern in global_patterns.items():
            match = re.search(pattern, content)
            if match:
                components[key] = match.group(1)
                #print(f"[INFO] Found {key}: {components[key]}")
        
        # 构建完整的版本字符串
        if len(components) == 6:
            # 按照APP_VERSION的格式拼接：MOUDLE_TYPE.PROJECT_CODE.SOFTWARE_VER.DEBUG_VER.RELEASE_DATE.HARDWARE_VER
            full_version = f"{components['MOUDLE_TYPE']}.{components['PROJECT_CODE']}.{components['SOFTWARE_VER']}.{components['DEBUG_VER']}.{components['RELEASE_DATE']}.{components['HARDWARE_VER']}"
            print(f"[INFO] Full APP version string: {full_version}")
            return full_version
        else:
            # 如果无法获取完整版本信息，使用默认名称
            missing_keys = [key for key in ['MOUDLE_TYPE', 'PROJECT_CODE', 'SOFTWARE_VER', 'DEBUG_VER', 'RELEASE_DATE', 'HARDWARE_VER'] if key not in components]
            print(f"[WARNING] Missing version components: {missing_keys}, using default name")
            return "update"
            
    except Exception as e:
        print(f"[WARNING] Failed to read version.c: {e}, using default name")
        return "update"


def create_update_package():
    """创建更新包"""
    
    # 获取当前脚本所在目录
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # 目标路径 - 项目根目录
    target_path = os.path.join(script_dir, '..')
    
    # 获取版本号作为文件夹名称
    folder_name = get_app_version()
    
    # 创建版本命名的文件夹路径 - 在output目录下创建
    update_dir = os.path.join(target_path, 'proj', 'output', folder_name)
    
    # 源文件路径 - 在proj/output目录中查找TZS100_VPU.bin文件
    source_file = None
    
    # First search in proj/output directory
    proj_output_dir = os.path.join(target_path, 'proj', 'output')
    if os.path.exists(proj_output_dir):
        #print(f"[INFO] Searching directory: {proj_output_dir}")
        
        # Check specific files
        potential_files = [
            os.path.join(proj_output_dir, 'TZS100_VPU.bin'),
            os.path.join(proj_output_dir, 'TZS100_VPU_Target.bin'),
        ]
        
        # Check specific filenames first
        for file_path in potential_files:
            if os.path.exists(file_path):
                source_file = file_path
                #print(f"[INFO] Found file: {file_path}")
                break
        
        # If specific files not found, search all .bin files
        if not source_file:
            import glob
            bin_files = glob.glob(os.path.join(proj_output_dir, '*.bin'))
            if bin_files:
                source_file = bin_files[0]  # Use first found .bin file
                print(f"[INFO] Found file using wildcard: {source_file}")
    
    # If not found in proj/output, try output directory
    if not source_file:
        output_dir = os.path.join(target_path, 'output')
        if os.path.exists(output_dir):
            #print(f"[INFO] Searching directory: {output_dir}")
            
            potential_files = [
                os.path.join(output_dir, 'TZS100_VPU.bin'),
                os.path.join(output_dir, 'TZS100_VPU_Target.bin'),
            ]
            
            for file_path in potential_files:
                if os.path.exists(file_path):
                    source_file = file_path
                    #print(f"[INFO] Found file: {file_path}")
                    break
            
            if not source_file:
                import glob
                bin_files = glob.glob(os.path.join(output_dir, '*.bin'))
                if bin_files:
                    source_file = bin_files[0]
                    print(f"[INFO] Found file using wildcard: {source_file}")
                    
                    # 如果在这个目录找到文件，版本命名的文件夹也应该创建在这里
                    update_dir = os.path.join(output_dir, folder_name)
    
    if not source_file:
        print("[ERROR] TZS100_VPU.bin file not found")
        return False
    
    try:
        # 清理所有版本文件夹
        output_parent_dir = os.path.join(target_path, 'proj', 'output')
        if os.path.exists(output_parent_dir):
            # 获取所有文件夹
            all_folders = [f for f in os.listdir(output_parent_dir) 
                          if os.path.isdir(os.path.join(output_parent_dir, f))]
            
            # 删除所有文件夹（包括当前版本，因为我们要重新创建）
            for folder_name in all_folders:
                folder_path = os.path.join(output_parent_dir, folder_name)
                #print(f"[INFO] Removing version folder: {folder_path}")
                shutil.rmtree(folder_path)
        
        # Create version-named folder
        os.makedirs(update_dir, exist_ok=True)
        #print(f"[INFO] Created version folder: {update_dir}")
        
        # Destination file path
        dest_file = os.path.join(update_dir, 'TZS100_VPU.bin')
        
        # Copy file
        shutil.copy2(source_file, dest_file)
        #print(f"[INFO] Copied file: {source_file} -> {dest_file}")
        
        # Get file info
        file_size = os.path.getsize(dest_file)
        #print(f"[INFO] File size: {file_size} bytes")
        
        # 3. 计算CRC32校验和
        #print("\n[INFO] Calculating CRC32 checksum...")
        crc_value = calculate_crc32(dest_file)
        #print(f"[INFO] CRC32 value: 0x{crc_value:08X}")
        
        # 4. 将CRC32值追加到文件末尾
        #print("[INFO] Appending CRC32 to file...")
        append_crc32_to_file(dest_file)
        
        # 5. 重命名文件为update_size.bin格式
        #print("[INFO] Renaming file...")
        final_path = rename_file_with_size(dest_file, "update")
        
        # 显示最终文件信息
        final_size = os.path.getsize(final_path)
        print(f"\n[INFO] Final file: {os.path.basename(final_path)}")
        print(f"[INFO] Final file size: {final_size} bytes")
        print(f"[INFO] CRC32 value: 0x{crc_value:08X}")
        
        return True
        
    except Exception as e:
        print(f"[ERROR] Failed to create update package: {e}")
        return False


def main():
    """Main function"""
    
    #print("Starting update package generation...")
    
    if create_update_package():
        print("\n[SUCCESS] Update package generation completed")
        return 0
    else:
        print("\n[FAILED] Update package generation failed")
        return 1


if __name__ == "__main__":
    sys.exit(main())