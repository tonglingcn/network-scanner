#!/bin/bash
# Git 仓库初始化脚本

echo "=================================="
echo "  网络扫描器 - Git 仓库初始化"
echo "=================================="
echo

# 检查是否在 Git 仓库中
if [ -d ".git" ]; then
    echo "⚠️  警告: 当前目录已经是 Git 仓库"
    read -p "是否继续? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    echo "📦 初始化 Git 仓库..."
    git init
fi

# 配置用户信息（如果未设置）
if [ -z "$(git config user.name)" ]; then
    echo "⚙️  请输入 Git 用户名: "
    read git_username
    git config user.name "$git_username"
fi

if [ -z "$(git config user.email)" ]; then
    echo "⚙️  请输入 Git 邮箱: "
    read git_email
    git config user.email "$git_email"
fi

echo
echo "✅ Git 配置信息:"
echo "   用户: $(git config user.name)"
echo "   邮箱: $(git config user.email)"
echo

# 添加所有文件
echo "📝 添加文件到暂存区..."
git add .

# 提交
echo "💾 创建初始提交..."
git commit -m "Initial commit: Network Scanner v2.0

- Qt6-based network scanner tool
- MAC address detection and vendor identification
- Multi-threaded IP scanning
- Export results to CSV
- MIT License
"

# 添加远程仓库
echo
echo "🔗 配置远程仓库..."
echo "请输入 GitHub 仓库地址 (默认: https://github.com/tonglingcn/network-scanner.git)"
read -r remote_url

if [ -z "$remote_url" ]; then
    remote_url="https://github.com/tonglingcn/network-scanner.git"
fi

git remote add origin "$remote_url"
git remote -v

echo
echo "✨ 初始化完成！"
echo
echo "下一步操作:"
echo "  1. 在 GitHub 上创建仓库: $remote_url"
echo "  2. 推送到 GitHub:"
echo "     git branch -M main"
echo "     git push -u origin main"
echo
