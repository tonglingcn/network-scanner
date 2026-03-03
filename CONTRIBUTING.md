# 贡献指南

感谢你对网络扫描器项目的关注！我们欢迎任何形式的贡献。

## 如何贡献

### 报告 Bug

如果你发现了 Bug，请：
1. 在 [Issues](https://github.com/tonglingcn/network-scanner/issues) 中搜索是否已有相同问题
2. 如果没有，创建新的 Issue，使用 "Bug 反馈" 模板
3. 提供详细的复现步骤和环境信息

### 提出功能建议

如果你有新功能想法：
1. 先在 Issues 中搜索是否已有类似建议
2. 创建新的 Issue，使用 "功能建议" 模板
3. 清晰描述功能的使用场景和期望行为

### 提交代码

1. **Fork 仓库**
   ```bash
   # 在 GitHub 上 Fork 本仓库
   # 然后克隆你的 Fork
   git clone https://github.com/你的用户名/network-scanner.git
   cd network-scanner
   ```

2. **创建分支**
   ```bash
   git checkout -b feature/你的功能名
   # 或者
   git checkout -b fix/修复的问题
   ```

3. **进行修改**
   - 遵循现有的代码风格
   - 添加必要的注释
   - 确保代码可以正常编译

4. **测试修改**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build . -j$(nproc)
   ./NetworkScanner
   ```

5. **提交更改**
   ```bash
   git add .
   git commit -m "简短的提交说明"
   ```

6. **推送到你的 Fork**
   ```bash
   git push origin feature/你的功能名
   ```

7. **创建 Pull Request**
   - 在 GitHub 上访问你的 Fork
   - 点击 "New Pull Request"
   - 填写 PR 模板中的必要信息

## 代码规范

### C++ 代码风格

- 使用 4 空格缩进
- 大括号另起一行
- 类名使用大驼峰命名（PascalCase）
- 函数和变量使用小驼峰命名（camelCase）
- 成员变量使用 `m_` 前缀
- 常量使用全大写下划线分隔（UPPER_SNAKE_CASE）

### 提交信息规范

使用清晰的提交信息格式：
```
<类型>: <简短描述>

<详细描述（可选）>
```

类型示例：
- `feat`: 新功能
- `fix`: Bug 修复
- `docs`: 文档更新
- `style`: 代码格式调整
- `refactor`: 代码重构
- `test`: 测试相关
- `chore`: 构建/工具相关

示例：
```
feat: 添加导出扫描结果为 CSV 文件功能

新增了导出按钮和文件保存对话框，
支持将扫描结果导出为 CSV 格式。
```

## 开发环境设置

### 依赖安装

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools cmake build-essential
```

### 编译项目

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

### 运行测试

```bash
cd build
./NetworkScanner
```

## 发布流程

发布新版本时，请：
1. 更新 `CMakeLists.txt` 中的版本号
2. 更新 `README.md` 中的版本历史
3. 更新 `debian/changelog`
4. 创建 Git tag: `git tag v2.0.1`
5. 推送 tag: `git push origin v2.0.1`

## 许可证

通过贡献代码，你同意你的贡献将采用 MIT 许可证进行授权。

## 联系方式

如有问题，请通过以下方式联系：
- GitHub Issues: https://github.com/tonglingcn/network-scanner/issues
- 邮箱: kel@example.com

---

再次感谢你的贡献！
