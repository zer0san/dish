# 终端增强功能实现计划

**Goal:** 为终端模拟器添加 Tab 自动补全、彩色输出、欢迎横幅功能。

**Architecture:** 在现有 TerminalWidget 类中添加新方法，不引入新文件。

---

### Task 1: 添加 appendColoredOutput 方法

**Files:**
- Modify: `terminal/TerminalWidget.h`
- Modify: `terminal/TerminalWidget.cpp`

- [ ] **Step 1.1: 添加头文件声明**

在 `TerminalWidget.h` 的 `appendOutput` 下方添加：
```cpp
void appendColoredOutput(const QString &text, const QColor &color);
```

- [ ] **Step 1.2: 实现方法**

在 `TerminalWidget.cpp` 中添加：
```cpp
void TerminalWidget::appendColoredOutput(const QString &text, const QColor &color) {
    moveCursor(QTextCursor::End);
    QTextCursor cursor = textCursor();
    QTextCharFormat format;
    format.setForeground(color);
    cursor.insertText(text + "\n", format);
    setTextCursor(cursor);
    ensureCursorInInputArea();
}
```

---

### Task 2: 添加欢迎横幅

**Files:**
- Modify: `terminal/TerminalWidget.h`
- Modify: `terminal/TerminalWidget.cpp`

- [ ] **Step 2.1: 添加头文件声明**

```cpp
void showWelcomeBanner();
```

- [ ] **Step 2.2: 实现 showWelcomeBanner**

```cpp
void TerminalWidget::showWelcomeBanner() {
    appendColoredOutput("  ____  _     _     _", QColor("#FFFFFF"));
    appendColoredOutput(" |  _ \\(_)___| |__ | |__   ___  _ __", QColor("#FFFFFF"));
    appendColoredOutput(" | | | | / __| '_ \\| '_ \\ / _ \\| '_ \\", QColor("#FFFFFF"));
    appendColoredOutput(" | |_| | \\__ \\ | | | | | | (_) | | | |", QColor("#FFFFFF"));
    appendColoredOutput(" |____/|_|___|_| |_|_|_|\\___/|_| |_|", QColor("#FFFFFF"));
    appendColoredOutput("", QColor("#FFFFFF"));
    appendColoredOutput(" dish - Linux File System Simulator v0.1", QColor("#00FF00"));
    appendColoredOutput(" Type 'help' for available commands.", QColor("#00FF00"));
    appendColoredOutput("", QColor("#00FF00"));
}
```

- [ ] **Step 2.3: 在构造函数中调用**

在 `initStyle()` 之后、`showPrompt()` 之前插入：
```cpp
showWelcomeBanner();
```

---

### Task 3: 实现 Tab 自动补全

**Files:**
- Modify: `terminal/TerminalWidget.h`
- Modify: `terminal/TerminalWidget.cpp`

- [ ] **Step 3.1: 添加头文件声明**

```cpp
QStringList m_commands;
void handleTab();
```

- [ ] **Step 3.2: 初始化命令列表**

在构造函数中添加：
```cpp
, m_commands({"login", "logout", "mkdir", "rmdir", "cd", "ls",
              "create", "delete", "open", "close", "read", "write",
              "seek", "chmod", "format", "stat", "pwd", "useradd", "help"})
```

- [ ] **Step 3.3: 实现 handleTab**

```cpp
void TerminalWidget::handleTab() {
    QString input = currentInput();
    QStringList parts = input.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    QString prefix = parts.first();
    QStringList matches;
    for (const QString &cmd : m_commands) {
        if (cmd.startsWith(prefix)) {
            matches.append(cmd);
        }
    }

    if (matches.isEmpty()) return;

    if (matches.size() == 1) {
        QString completion = matches.first();
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, m_promptLength);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertText(completion + " ");
        setTextCursor(cursor);
    } else {
        moveCursor(QTextCursor::End);
        QTextCursor cursor = textCursor();
        cursor.insertText("\n");
        setTextCursor(cursor);
        for (const QString &m : matches) {
            appendColoredOutput("  " + m, QColor("#FFFF55"));
        }
        showPrompt();
        cursor = textCursor();
        cursor.insertText(input);
        setTextCursor(cursor);
    }
}
```

- [ ] **Step 3.4: 修改 Tab 键处理**

将 `keyPressEvent` 中的 Tab 拦截改为调用 `handleTab()`：
```cpp
if (event->key() == Qt::Key_Tab) {
    handleTab();
    return;
}
```

---

### Task 4: 构建验证

- [ ] **Step 4.1: 编译**
```bash
cmake --build cmake-build-debug
```
