# 终端模拟器 UI 设计文档

## 概述

为 OS 文件系统模拟项目实现一个经典 Linux 风格的终端模拟器界面，基于 Qt6 `QPlainTextEdit` 子类化。当前阶段仅实现 UI 框架，命令处理为模拟响应。

## 文件结构

```
dish/
├── CMakeLists.txt          (修改：添加新源文件)
├── main.cpp                (修改：替换按钮为终端窗口)
├── terminal/
│   ├── TerminalWidget.h    (终端主控件)
│   ├── TerminalWidget.cpp
│   ├── TerminalInput.h     (输入处理 + 命令历史)
│   └── TerminalInput.cpp
└── info.md
```

## 类设计

### TerminalWidget

继承 `QPlainTextEdit`，负责终端显示和交互。

**公有方法：**

```cpp
void appendOutput(const QString &text);   // 输出一行文本（只读区域）
void setPrompt(const QString &prompt);    // 设置提示符
QString currentInput() const;             // 获取当前输入内容
void clear();                             // 清屏
```

**信号：**

```cpp
void commandEntered(const QString &command);  // 用户按下 Enter 时发射
```

**核心逻辑：**

- 所有通过 `appendOutput()` 写入的文本自动设为只读区域
- 用户只能在最后一行（提示符之后）输入
- 每次按键检查光标位置，不在输入区域时强制移回末尾
- 提示符格式：`user@dish:~/path$`

### TerminalInput

负责命令行编辑逻辑和命令历史管理。

**职责：**

- 命令历史存储（`QStringList`）
- 上下键翻转历史
- 预留 Tab 补全接口

## 视觉风格

| 属性 | 值 |
|------|------|
| 背景色 | `#000000`（纯黑） |
| 文字颜色 | `#00FF00`（绿色） |
| 提示符颜色 | `#FFFFFF`（白色粗体） |
| 字体 | `Courier New` 或 `Consolas`，14px |
| 光标 | 块状光标，闪烁 |
| 窗口大小 | 800×500，默认居中 |

样式通过 `setStyleSheet()` 和 `QFont` 设置。

## 按键处理

| 按键 | 行为 |
|------|------|
| Enter | 提取输入 → 发射信号 → 回显输入到输出区 → 清空输入行 → 显示新提示符 |
| Backspace | 正常删除，不能删除到提示符之前 |
| 上/下键 | 命令历史翻转 |
| Ctrl+C | 取消当前输入，换行显示新提示符 |
| Ctrl+L | 清屏，保留当前提示符 |
| 其他 | 正常输入，只允许在最后一行编辑 |

## 防篡改机制

- `keyPressEvent` 中检查光标位置，不在输入区域时强制移回末尾
- 非输入区域设置 `setReadOnly(true)` 禁止编辑

## 与文件系统后端的接口预留

当前阶段终端发射的 `commandEntered` 信号不连接任何槽。后续文件系统实现后，将信号连接到命令解析器即可完成集成。
