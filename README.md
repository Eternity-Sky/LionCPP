# 小狮子C++ IDE

一个功能完整的中文C++集成开发环境，基于Qt框架开发。

## 功能特性

### 代码编辑器
- ✅ 语法高亮（C++关键字、注释、字符串等）
- ✅ 行号显示
- ✅ 代码补全（Ctrl+Space）
- ✅ 当前行高亮
- ✅ 自动缩进
- ✅ 多标签页编辑
- ✅ 文件拖拽支持

### 项目管理
- ✅ 创建新项目
- ✅ 打开现有项目
- ✅ 项目文件树视图
- ✅ 添加/删除文件
- ✅ 文件重命名
- ✅ 项目配置保存

### 编译和运行
- ✅ CMake项目支持
- ✅ 自动生成CMakeLists.txt
- ✅ 编译输出显示
- ✅ 程序运行
- ✅ 构建清理
- ✅ 错误信息显示

### 用户界面
- ✅ 中文界面
- ✅ 可停靠窗口
- ✅ 工具栏
- ✅ 状态栏
- ✅ 菜单栏
- ✅ 设置对话框

### 其他功能
- ✅ 文件保存/另存为
- ✅ 撤销/重做
- ✅ 剪切/复制/粘贴
- ✅ 设置保存
- ✅ 窗口状态记忆

## 系统要求

- Qt 6.x 或 Qt 5.x
- CMake 3.14+
- C++17 编译器（GCC、Clang、MSVC）
- Linux/macOS/Windows

## 编译和安装

### 依赖安装

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install qt6-base-dev cmake build-essential
```

#### CentOS/RHEL:
```bash
sudo yum install qt6-qtbase-devel cmake gcc-c++
```

#### macOS:
```bash
brew install qt cmake
```

#### Windows:
- 安装 Qt 6.x
- 安装 CMake
- 安装 Visual Studio 或 MinGW

### 编译步骤

1. 克隆项目：
```bash
git clone https://github.com/your-repo/lioncpp-ide.git
cd lioncpp-ide
```

2. 创建构建目录：
```bash
mkdir build
cd build
```

3. 配置项目：
```bash
cmake ..
```

4. 编译：
```bash
make -j$(nproc)
```

5. 运行：
```bash
./LionCPP
```

## 使用指南

### 创建新项目

1. 点击"文件" → "新建项目"
2. 输入项目名称
3. 选择项目目录
4. 点击确定

IDE会自动创建：
- `main.cpp` - 主程序文件
- `CMakeLists.txt` - CMake配置文件
- 项目配置文件

### 编辑代码

1. 双击项目树中的文件打开编辑器
2. 使用语法高亮和代码补全功能
3. 按 Ctrl+Space 触发代码补全
4. 使用 Ctrl+S 保存文件

### 编译和运行

1. 点击"构建" → "构建"或按 Ctrl+Shift+B
2. 等待编译完成
3. 点击"构建" → "运行"或按 Ctrl+R
4. 查看输出窗口的编译和运行信息

### 项目管理

- 右键点击项目树添加新文件
- 右键点击文件进行重命名或删除
- 拖拽文件到项目中

### 设置

点击"工具" → "设置"打开设置对话框：

- **编辑器**：字体、颜色、选项设置
- **编译器**：编译器路径、构建选项
- **通用**：自动备份、更新检查等

## 📝 作者

- **开发者**: Eternity-Sky
- **GitHub**: [Eternity-Sky](https://github.com/Eternity-Sky)
- **B站**: [WIN_CTRL](https://space.bilibili.com/WIN_CTRL)
- **版本**: 1.0.0

## 快捷键

| 功能 | 快捷键 |
|------|--------|
| 新建项目 | Ctrl+N |
| 打开项目 | Ctrl+O |
| 保存 | Ctrl+S |
| 另存为 | Ctrl+Shift+S |
| 新建文件 | Ctrl+Shift+N |
| 打开文件 | Ctrl+O |
| 关闭文件 | Ctrl+W |
| 撤销 | Ctrl+Z |
| 重做 | Ctrl+Y |
| 剪切 | Ctrl+X |
| 复制 | Ctrl+C |
| 粘贴 | Ctrl+V |
| 查找 | Ctrl+F |
| 替换 | Ctrl+H |
| 构建 | Ctrl+Shift+B |
| 运行 | Ctrl+R |
├── projectmanager.cpp    # 项目管理器实现
├── compiler.h            # 编译器头文件
├── compiler.cpp          # 编译器实现
├── settingsdialog.h      # 设置对话框头文件
├── settingsdialog.cpp    # 设置对话框实现
├── settingsdialog.ui     # 设置对话框UI文件
├── lioncpp.qrc           # 资源文件
├── CMakeLists.txt        # CMake配置
├── icons/                # 图标目录
└── README.md             # 说明文档
```

## 开发计划

### 即将添加的功能
- [ ] 调试器集成
- [ ] 代码格式化
- [ ] 代码折叠
- [ ] 多主题支持
- [ ] 插件系统
- [ ] 版本控制集成
- [ ] 代码片段
- [ ] 智能提示
- [ ] 错误检查
- [ ] 性能分析

### 已知问题
- 某些Qt版本可能存在兼容性问题
- 大型项目编译可能较慢
- 图标文件需要手动添加

## 贡献

欢迎提交Issue和Pull Request！

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 联系方式

- 项目主页：https://github.com/your-repo/lioncpp-ide
- 问题反馈：https://github.com/your-repo/lioncpp-ide/issues
- 邮箱：your-email@example.com

---

**小狮子C++ IDE** - 让C++编程更简单！ 