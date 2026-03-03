#!/bin/bash
set -e
echo "=================================="
echo "网络扫描器 (Network Scanner) v1.0"
echo "构建脚本"
echo "=================================="
echo ""
echo "检查依赖..."
if ! command -v cmake &> /dev/null; then
    echo "错误: 未找到 cmake"
    exit 1
fi
echo "依赖检查通过"
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "配置项目..."
cmake .. -DCMAKE_BUILD_TYPE=Release
echo "编译项目..."
cmake --build . -j$(nproc)
echo "构建完成！"
echo "运行: cd build && ./NetworkScanner"
