# 终端增强功能设计文档

## 概述

为终端模拟器添加三个高优先级功能：Tab 自动补全、彩色输出、欢迎横幅。

## 功能 1：Tab 自动补全

**当前阶段：** 仅补全命令名（文件系统未实现，无法补全路径）

**命令列表（17 个）：**
login, logout, mkdir, rmdir, cd, ls, create, delete, open, close, read, write, seek, chmod, format, stat, pwd, useradd

**行为：**
- 按 Tab 时，提取当前输入的第一个单词作为命令前缀
- 匹配所有以该前缀开头的命令
- 仅一个匹配 → 直接补全（加空格）
- 多个匹配 → 显示所有匹配项，重新显示提示符+当前输入
- 无匹配 → 不做任何事

**实现位置：** `TerminalWidget::keyPressEvent` 中处理 Tab 键

## 功能 2：彩色输出

**接口：**
```cpp
void appendColoredOutput(const QString &text, const QColor &color);
```

**颜色定义：**
| 类型 | 颜色 | 用途 |
|------|------|------|
| 绿色 | `#00FF00` | 默认正常输出 |
| 红色 | `#FF5555` | 错误（命令未找到、操作失败） |
| 白色 | `#FFFFFF` | 系统信息、帮助 |
| 黄色 | `#FFFF55` | 警告（权限不足等） |

**实现：** 使用 QTextCursor + QTextCharFormat 插入带颜色文本，与现有 `showPrompt()` 方法的模式一致。

## 功能 3：欢迎横幅

**内容：**
```
  ____  _     _     _
 |  _ \(_)___| |__ | |__   ___  _ __
 | | | | / __| '_ \| '_ \ / _ \| '_ \
 | |_| | \__ \ | | | | | | (_) | | | |
 |____/|_|___|_| |_|_| |_|\___/|_| |_|

 dish - Linux File System Simulator v0.1
 Type 'help' for available commands.
```

**实现：** 在 `TerminalWidget` 构造函数中调用 `showWelcomeBanner()`，用 `appendColoredOutput` 以白色显示 ASCII art，绿色显示说明文字。

## 修改文件

| 文件 | 修改内容 |
|------|----------|
| `terminal/TerminalWidget.h` | 添加方法声明 |
| `terminal/TerminalWidget.cpp` | 实现三个功能 |
