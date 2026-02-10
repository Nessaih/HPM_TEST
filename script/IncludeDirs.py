import os
import fnmatch


def print_include_dirs(project_path, base_path):
    for rootpath, dirnames, filenames in os.walk(base_path):
        if fnmatch.filter(filenames, '*.h'):
            relative_path = f'$PROJ_DIR$\\{os.path.relpath(rootpath, project_path)}'
            print(relative_path)


if '__main__' == __name__:
    root_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    project_path = os.path.join(root_path, 'proj\\iar')

    print(f"工程根路径  ：{root_path}")
    print(f"项目文件路径：{project_path}")

    print("\n头文件路径：")
    print_include_dirs(project_path, root_path)
