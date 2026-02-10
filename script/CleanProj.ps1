

<##
  ###############################################################################
  # @file        clean_proj.ps1
  # @path        script\clean_proj.ps1
  # @brief       1.运行此脚本前，请先关闭IAR工程;
  #              2. ...
  # @record
  # @Change Logs:
  # Date             Author          Notes
  # 2024-12-02       vic             First version
  ###############################################################################
  #>

$proj_dir = "$PSScriptRoot\..\proj\iar"

$filesToDelete = @("*.dep", "*.ewt", "Debug", "Release", "Settings")


Write-Host "删除文件夹中..."

foreach ($f in $filesToDelete) {
    $path = $proj_dir + "\" + $f

    if (Test-Path $path) {
        Write-Host "正在删除: $path"
        Remove-Item -Path $path -Recurse -Force
    }
}

Write-Host "删除完成！"

