# 网络扫描器 (Network Scanner)

一款基于 Qt6 + C++ 开发的局域网 IP 地址扫描工具，可快速检测局域网内 IP 地址的使用情况，并获取详细的设备信息。

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Stars](https://img.shields.io/github/stars/tonglingcn/network-scanner?style=social)](https://github.com/tonglingcn/network-scanner)

**项目地址**: https://github.com/tonglingcn/network-scanner

## 功能特点

- **快速扫描**: 支持快速扫描整个网段 (默认 1-254)
- **自定义范围**: 可自定义扫描 IP 范围
- **多线程扫描**: 使用线程池并发扫描，性能优异
- **实时进度**: 实时显示扫描进度和统计信息
- **状态显示**: 直观显示 IP 空闲/占用状态，带颜色标识
- **响应时间**: 显示已占用 IP 的响应时间
- **结果导出**: 支持导出扫描结果为 CSV 文件
- **网络接口自动检测**: 自动识别本地网络接口和网段配置

### 新增功能 (v2.0)

- **MAC 地址解析**: 自动获取在线设备的 MAC 地址
- **厂商识别**: 根据 MAC 地址识别设备厂商
- **主机名解析**: 通过 DNS 反向查询获取主机名
- **网卡厂商**: 显示网卡硬件厂商信息

## 技术栈

- Qt 6 (Core, Widgets, Network, Concurrent)
- C++17
- CMake 构建系统
- Linux ping 命令 (ICMP)
- ARP 协议用于 MAC 地址解析
- DNS 反向查询用于主机名解析

## 技术栈

- Qt 6 (Core, Widgets, Network, Concurrent)
- C++17
- CMake 构建系统
- Linux ping 命令 (ICMP)

## 系统要求

- Linux 操作系统 (已测试: Ubuntu 20.04+, Debian 11+, Deepin)
- Qt 6.2 或更高版本
- CMake 3.16 或更高版本
- C++17 兼容编译器 (GCC 8+, Clang 7+)

## 依赖安装

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools cmake build-essential
```

### Deepin
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools cmake build-essential
```

## 编译说明

### 1. 创建构建目录
```bash
mkdir build && cd build
```

### 2. 配置项目
```bash
cmake ..
```

### 3. 编译
```bash
cmake --build . -j$(nproc)
```

### 4. 安装 (可选)
```bash
sudo cmake --install .
```

## 使用说明

### 快速扫描
1. 启动程序，选择要扫描的网络接口
2. 点击"快速扫描整个网段"按钮
3. 等待扫描完成，查看结果

### 自定义范围扫描
1. 在扫描设置中设置网络地址和子网掩码
2. 设置起始 IP 和结束 IP
3. 点击"自定义范围扫描"按钮

### 查看结果
扫描结果表格包含以下信息：
- **IP 地址**: 扫描的 IP 地址
- **状态**: "已占用"（红色）或 "空闲"（绿色）
- **响应时间**: 已占用设备的响应时间（毫秒）
- **MAC 地址**: 设备的物理地址（仅在 IP 已占用时显示）
- **厂商**: 根据 MAC 地址前缀识别的设备厂商
- **主机名**: 通过 DNS 反向查询获取的主机名
- **网卡厂商**: 网卡硬件厂商

### 导出结果
扫描完成后，点击"导出结果"按钮，选择保存位置和格式（CSV）。

## 信息解析说明

### MAC 地址获取
- 通过 ARP 协议查询 IP 对应的 MAC 地址
- 先发送 ping 请求触发 ARP 响应
- 然后从系统 ARP 表中查询对应的 MAC 地址

### 厂商识别
- 使用 OUI (Organizationally Unique Identifier) 数据库
- 根据 MAC 地址的前 3 字节（前缀）识别厂商
- 内置了常见厂商的 OUI 数据库

### 主机名解析
- 使用 DNS 反向查询（PTR 记录）
- 通过 `QHostInfo::fromName()` 进行查询
- 如果 DNS 服务器配置正确，可以获取主机名

### 注意事项
- MAC 地址获取需要 root 权限或使用 sudo 运行
- 主机名解析依赖于网络中的 DNS 服务器配置
- 某些设备可能禁用了 ICMP 响应，导致无法检测
- 虚拟机的 MAC 地址前缀通常显示虚拟化厂商名称

## 权限说明

在 Linux 上，使用 ICMP ping 命令扫描网络通常不需要特殊权限。但如果遇到权限问题，可以尝试:

```bash
sudo setcap cap_net_raw+ep /path/to/NetworkScanner
```

或直接使用 sudo 运行:
```bash
sudo ./NetworkScanner
```

## 项目结构

```
net-tool/
├── CMakeLists.txt           # CMake 构建配置
├── README.md                # 项目说明
├── src/
│   ├── main.cpp            # 程序入口
│   ├── mainwindow.h/cpp    # 主窗口界面
│   ├── mainwindow.ui       # UI 设计文件
│   ├── ipscanner.h/cpp     # 扫描器核心逻辑
│   ├── ipscannerthread.h/cpp  # 扫描工作线程
│   └── scannermodel.h/cpp  # 数据模型
└── assets/
    ├── network-scanner.desktop  # Linux 桌面快捷方式
    └── icon.png            # 应用图标
```

## 性能优化

- 使用线程池并发扫描，默认使用 `CPU核心数 * 2` 个线程
- 每个主机 ping 超时设置为 1 秒，快速完成扫描
- 支持批量扫描，扫描 254 个 IP 通常在 1-2 分钟内完成

## 已知限制

- 仅支持 IPv4 地址扫描
- 需要在 Linux 系统上运行
- 扫描速度受网络环境影响

## 许可证

本项目采用 MIT 许可证开源。详见 [LICENSE](LICENSE) 文件。

## 作者

克亮

## 相关链接

- [GitHub 仓库](https://github.com/tonglingcn/network-scanner)
- [提交问题](https://github.com/tonglingcn/network-scanner/issues)
- [贡献代码](https://github.com/tonglingcn/network-scanner/pulls)
- [更新日志](CHANGELOG.md)
- [贡献指南](CONTRIBUTING.md)

## 版本历史

### v2.0 (2026-03-03)
- 新增 MAC 地址解析功能
- 新增厂商识别功能（基于 OUI 数据库）
- 新增主机名解析功能（DNS 反向查询）
- 新增网卡厂商信息显示
- 扩展扫描结果表格显示更多设备信息
- 优化扫描算法和性能

### v1.0 (2026-03-03)
- 初始版本发布
- 支持基本扫描功能
- 支持 UI 显示和结果导出
