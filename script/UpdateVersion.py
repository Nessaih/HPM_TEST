import re
import sys
import os
import datetime


def today():
    date = datetime.datetime.now().strftime('%Y%m%d')
    return date


def update(path, date):

    # 读取文件内容
    with open(path, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    # 找到匹配行并替换
    p = re.compile(r'(RELEASE_DATE\s+)"(\w+)"')
    for i, l in enumerate(lines):
        obj = re.search(p, l)
        if obj and obj.group(2) != date:
            lines[i] = re.sub(p, r'\1"' + date + '"', l)
            # 写回替换后的字符串
            with open(path, 'w', encoding='utf-8') as file:
                file.writelines(lines)
            break
    
    os.utime(path, None) # Update file timestamp

if '__main__' == __name__:

    if len(sys.argv) == 2:
        update(sys.argv[1], today())

    # update(r"D:\Code\Work\TEST-DEMO\AC7840x\src\version\version.c", today())
