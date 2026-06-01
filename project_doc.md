# dish - Linux Terminal Simulator 项目文档

## 一、项目概述

**dish** 是一个基于 Qt6 的 Linux 终端模拟器，模拟了 UNIX 文件系统的核心功能。该项目实现了一个完整的虚拟文件系统，支持多用户管理、多级目录结构、文件操作等功能。

**核心目标**：提供一个类似 Linux 的命令行界面，让用户体验文件系统操作。

## 二、技术架构

### 2.1 技术栈

| 分类 | 技术 | 版本 |
|------|------|------|
| 框架 | Qt | 6.x |
| 语言 | C++ | C++17 |
| 构建工具 | CMake | 3.31+ |

### 2.2 整体架构层次

```
┌─────────────────────────────────────────────────────┐
│                   UI 层 (Qt6)                       │
│         TerminalWidget / TerminalInput              │
├─────────────────────────────────────────────────────┤
│                 命令解析层                          │
│            CommandParser (命令分发)                 │
├─────────────────────────────────────────────────────┤
│                 文件系统 API 层                     │
│        FileSystem (mkdir, rmdir, cd, ls...)        │
├─────────────────────────────────────────────────────┤
│                 虚拟文件系统层                      │
│      Inode / Dentry / SuperBlock 管理               │
├─────────────────────────────────────────────────────┤
│                   物理存储层                        │
│              磁盘镜像文件 (.img)                    │
└─────────────────────────────────────────────────────┘
```

## 三、模块划分与功能

### 3.1 文件系统模块 (`fs/`)

| 文件 | 功能描述 |
|------|----------|
| `fs.hpp` / `fs.cpp` | 文件系统主类，提供文件和目录操作 API |
| `inode.hpp` / `inode.cpp` | Inode 结构定义与管理（文件元数据） |
| `dentry.hpp` / `dentry.cpp` | 目录项结构定义（文件名与 inode 映射） |
| `superblock.hpp` / `superblock.cpp` | 超级块结构（文件系统元信息） |

**核心数据结构**：

- **Inode**：存储文件元信息（权限、大小、时间戳、数据块指针）
- **Dentry**：目录项（文件名 + inode 编号映射）
- **SuperBlock**：文件系统全局元信息（魔数、块大小、空闲块数等）

**支持的操作**：
- 文件操作：create、delete、read、write、truncate
- 目录操作：mkdir、rmdir、ls、路径解析
- 权限管理：checkPermission

### 3.2 用户系统模块 (`user/`)

| 文件 | 功能描述 |
|------|----------|
| `userSystem.hpp` / `userSystem.cpp` | 用户系统管理主类 |
| `user.hpp` / `user.cpp` | 普通用户类 |
| `admin.hpp` / `admin.cpp` | 管理员用户类 |

**功能特性**：
- 用户注册与登录
- 管理员注册与登录
- 用户数据持久化存储
- 密码加密（QCryptographicHash）
- 双用户体系（普通用户 + 管理员）

### 3.3 终端模块 (`terminal/`)

| 文件 | 功能描述 |
|------|----------|
| `TerminalWidget.h` / `TerminalWidget.cpp` | Qt 终端界面组件 |
| `TerminalInput.h` / `TerminalInput.cpp` | 输入处理组件 |
| `CommandParser.h` / `CommandParser.cpp` | 命令解析器 |

**支持的命令**：

| 命令 | 功能 | 权限要求 |
|------|------|----------|
| `login` / `logout` | 用户登录/登出 | 无 |
| `register` | 用户注册 | 无 |
| `whoami` | 显示当前用户 | 无 |
| `useradd` | 添加用户 | 管理员 |
| `mkdir` | 创建目录 | 需写权限 |
| `rmdir` | 删除目录 | 需写权限 |
| `cd` | 切换目录 | 无 |
| `ls` | 列出目录 | 需读权限 |
| `create` | 创建文件 | 需写权限 |
| `delete` | 删除文件 | 需写权限 |
| `read` | 读取文件 | 需读权限 |
| `write` | 写入文件 | 需写权限 |
| `chmod` | 修改权限 | 所有者/管理员 |
| `format` | 格式化文件系统 | 管理员 |
| `stat` | 查看文件信息 | 需读权限 |
| `pwd` | 显示当前路径 | 无 |
| `mount` / `unmount` | 挂载/卸载文件系统 | 管理员 |
| `mv` | 移动文件/目录 | 需写权限 |
| `mkimg` | 创建磁盘镜像 | 管理员 |

### 3.4 初始化模块 (`init.hpp` / `init.cpp`)

负责系统启动时的初始化工作：
- 初始化数据目录路径
- 挂载/格式化文件系统
- 创建标准目录结构

## 四、文件系统持久化方案

### 4.1 磁盘镜像布局

```
+------------------+
|    SuperBlock    |  块 0
+------------------+
|   inode Bitmap   |  块 1
+------------------+
|   block Bitmap   |  块 2
+------------------+
|    inode 区      |  块 3 ~ 块 N
+------------------+
|    数据块区      |  块 N+1 ~ 最后
+------------------+
```

### 4.2 关键参数

| 参数 | 值 |
|------|-----|
| 块大小 | 4096 字节 |
| 魔数 | 0x12345678 |
| Dentry 大小 | 256 字节 |
| 文件名最大长度 | 250 字节 |

## 五、项目文件结构

```
dish/
├── .idea/              # CLion 项目配置
├── build/              # CMake 构建目录
├── docks/              # 文档目录
│   ├── info.md         # 项目设计文档
│   ├── LH.md           # 日志/历史记录
│   └── 文件系统设计.md  # 文件系统详细设计
├── fs/                 # 文件系统模块
│   ├── dentry.cpp/hpp
│   ├── fs.cpp/hpp
│   ├── inode.cpp/hpp
│   └── superblock.cpp/hpp
├── terminal/           # 终端模块
│   ├── CommandParser.cpp/h
│   ├── TerminalInput.cpp/h
│   └── TerminalWidget.cpp/h
├── user/               # 用户系统模块
│   ├── admin.cpp/hpp
│   ├── user.cpp/hpp
│   └── userSystem.cpp/hpp
├── init.cpp/hpp        # 初始化模块
├── main.cpp            # 程序入口
├── CMakeLists.txt      # CMake 配置
└── readme.md           # 项目说明
```

## 六、运行流程

```
1. QApplication 初始化
2. Init::initDataPath() - 初始化数据目录
3. UserSystem 初始化（加载用户数据）
4. FileSystem 初始化（挂载/格式化磁盘镜像）
5. Init::initStandardDirs() - 创建标准目录
6. TerminalWidget 启动，显示欢迎界面
7. 用户输入命令 → CommandParser 解析 → 执行操作 → 返回结果
8. 程序退出时卸载文件系统
```

## 七、安全特性

1. **密码加密**：使用 QCryptographicHash 进行密码加密存储
2. **权限控制**：基于 inode 的权限位进行读写执行权限检查
3. **管理员隔离**：敏感操作（format、useradd）仅管理员可执行
4. **根用户机制**：root 用户拥有最高权限

## 八、总结

**dish** 是一个功能完整的 Linux 终端模拟器，实现了：

- **完整的虚拟文件系统**：支持 inode、目录项、超级块管理
- **多用户系统**：普通用户与管理员分离
- **丰富的命令集**：20+ 个常用文件系统命令
- **Qt6 GUI 界面**：提供类终端的交互体验
- **数据持久化**：通过磁盘镜像文件保存所有数据

该项目适合用于学习 UNIX 文件系统原理、操作系统课程实践或作为嵌入式文件系统的原型参考。
