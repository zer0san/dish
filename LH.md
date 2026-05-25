# 命令行界面实现说明

## 架构

```
terminal/
├── TerminalWidget.h/cpp   终端显示 + 交互（继承 QPlainTextEdit）
├── TerminalInput.h/cpp     命令历史管理
├── CommandParser.h/cpp     命令解析 + 分发
```

## TerminalWidget

终端主控件，负责 UI 渲染和按键处理。

| 功能 | 说明 |
|------|------|
| 提示符 | `user@dish:~$`，白色粗体 |
| 输出 | `appendOutput()` 绿色 / `appendColoredOutput()` 自定义颜色 |
| 光标限制 | 输入只能在提示符之后，Home/End/鼠标点击均约束在输入区域 |
| 按键 | Enter 执行、Backspace 受限删除、上下键翻历史、Ctrl+C 取消、Ctrl+L 清屏 |
| Tab 补全 | 匹配命令名，单选直接补全，多选列出候选项 |
| 欢迎横幅 | 启动时显示 ASCII art + 版本信息 |

视觉风格：黑底 `#000000`、绿字 `#00FF00`、Consolas 14px、800x500 窗口。

## TerminalInput

命令历史存储，支持上下键翻转，空命令不记录。

## CommandParser

`execute(input)` 流程：解析 → 验证命令名 → 验证参数数量 → 分发执行。

返回 `CommandResult`（输出文本 + 类型），颜色自动映射：

| 类型 | 颜色 | 场景 |
|------|------|------|
| Success | `#00FF00` | 正常输出 |
| Error | `#FF5555` | 未知命令、参数错误 |
| Warning | `#FFFF55` | 权限不足等 |
| Info | `#FFFFFF` | help、系统信息 |

## 支持的命令

| 命令 | 参数 | 说明 |
|------|------|------|
| `help` | - | 显示所有命令 |
| `clear` | - | 清屏 |
| `login` | `<user>` | 用户登录 |
| `logout` | - | 用户登出 |
| `mkdir` | `<path>` | 创建目录 |
| `rmdir` | `<path>` | 删除空目录 |
| `cd` | `<path>` | 切换目录 |
| `ls` | `[-l]` | 列出目录内容 |
| `create` | `<file>` | 创建文件 |
| `delete` | `<file>` | 删除文件 |
| `open` | `<file> <mode>` | 打开文件 |
| `close` | `<fd>` | 关闭文件 |
| `read` | `<fd> <size>` | 读取文件 |
| `write` | `<fd> <text>` | 写入文件 |
| `seek` | `<fd> <offset>` | 文件指针定位 |
| `chmod` | `<file> <mode>` | 修改权限 |
| `format` | - | 格式化文件系统 |
| `stat` | `<path>` | 查看文件/目录信息 |
| `pwd` | - | 显示当前路径 |
| `useradd` | `<name>` | 添加用户（需root） |

所有命令当前为 stub 实现，后续接入文件系统后端时替换 `CommandParser::cmdXxx()` 方法体即可。
