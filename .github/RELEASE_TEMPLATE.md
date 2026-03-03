# Network Scanner v{版本号}

## 🎉 新功能

- 新功能 1 描述
- 新功能 2 描述

## 🐛 Bug 修复

- Bug 1 修复描述
- Bug 2 修复描述

## 🚀 改进

- 改进 1 描述
- 改进 2 描述

## 📦 安装

### 从源码编译

```bash
git clone https://github.com/tonglingcn/network-scanner.git
cd network-scanner
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
sudo cmake --install .
```

### Debian/Ubuntu 包

下载 `.deb` 包后安装：

```bash
sudo dpkg -i network-scanner_{版本号}-1_amd64.deb
```

## 🔧 系统要求

- Linux 操作系统 (Ubuntu 20.04+, Debian 11+, Deepin)
- Qt 6.2 或更高版本
- CMake 3.16 或更高版本

## 📝 变更日志

详见 [CHANGELOG.md](CHANGELOG.md)

## ⚠️ 注意事项

- MAC 地址解析需要 root 权限或使用 sudo 运行
- 主机名解析依赖于网络中的 DNS 服务器配置

## 🙏 致谢

感谢所有贡献者的支持！

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE)
