# 变更日志 (Changelog)

本文档记录了网络扫描器项目的所有重要变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 计划中
- 添加 IPv6 支持
- 添加端口扫描功能
- 添加设备详情查看功能
- 添加扫描历史记录

## [2.0.0] - 2026-03-03

### 新增
- MAC 地址自动解析功能
- 设备厂商识别（基于 OUI 数据库，支持 200+ 厂商）
- 主机名 DNS 反向查询
- 网卡硬件厂商信息显示
- 网络接口自动检测
- 扫描结果实时统计
- 关于对话框，显示版权信息
- 完整的 Debian 打包支持

### 改进
- 优化 ARP 请求逻辑，提高 MAC 地址获取成功率
- 添加多次重试机制
- 改进主机名解析准确性
- 优化扫描性能
- UI 界面优化，移除不可靠的主机名列

### 技术改进
- 使用 QNetworkInterface 获取本机 MAC 地址
- 实现 getLocalMacForIp 方法
- 扩展 OUI 数据库覆盖常见厂商
- 添加 ping+ARP 组合获取机制

## [1.0.0] - 2026-03-03

### 新增
- 初始版本发布
- 基本网络扫描功能
- IP 状态检测（空闲/占用）
- 响应时间测量
- 自定义扫描范围
- 扫描结果导出（CSV 格式）
- 多线程并发扫描
- 实时进度显示
- Qt6 界面

## [Unreleased]: https://github.com/tonglingcn/network-scanner/compare/v2.0.0...HEAD
## [2.0.0]: https://github.com/tonglingcn/network-scanner/compare/v1.0.0...v2.0.0
## [1.0.0]: https://github.com/tonglingcn/network-scanner/releases/tag/v1.0.0
