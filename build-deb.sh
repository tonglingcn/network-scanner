#!/bin/bash

# Network Scanner Debian Build Script
# 用于构建 .deb 包

set -e

VERSION="2.0"
PACKAGE_NAME="network-scanner"
DISTRO=$(lsb_release -sc 2>/dev/null || echo "unstable")

echo "=========================================="
echo "Network Scanner Debian 构建脚本"
echo "=========================================="
echo ""

# 检查依赖
echo "检查构建依赖..."
if ! command -v dpkg-buildpackage &> /dev/null; then
    echo "错误: dpkg-buildpackage 未安装"
    echo "请运行: sudo apt-get install dpkg-dev"
    exit 1
fi

if ! command -v lsb_release &> /dev/null; then
    echo "警告: lsb_release 未安装，跳过发行版信息"
fi

# 清理旧的构建产物
echo "清理旧的构建产物..."
rm -f ../*.deb ../*.dsc ../*.changes ../*.tar.gz ../*.buildinfo
rm -rf debian/network-scanner

# 设置环境变量
export DEB_BUILD_OPTIONS="parallel=$(nproc)"

# 构建源码包和二进制包
echo "开始构建 Debian 包..."
echo "版本: ${VERSION}"
echo "并行编译: $(nproc) 核心"
echo ""

if dpkg-buildpackage -b -us -uc -j$(nproc); then
    echo ""
    echo "=========================================="
    echo "构建成功！"
    echo "=========================================="
    echo ""
    echo "构建产物位于: ../*.deb"
    echo ""
    echo "要安装包，请运行:"
    echo "  sudo dpkg -i ../${PACKAGE_NAME}_${VERSION}-1_*.deb"
    echo ""
    echo "如果遇到依赖问题，请运行:"
    echo "  sudo apt-get install -f"
    echo ""
else
    echo ""
    echo "=========================================="
    echo "构建失败！"
    echo "=========================================="
    echo ""
    echo "请检查错误信息并修复问题"
    exit 1
fi
