# 终端模拟器 UI 实现计划

> **For agentic workers:** 按照本计划逐任务实现。每个步骤使用 checkbox (`- [ ]`) 跟踪进度。

**Goal:** 实现一个经典 Linux 风格的终端模拟器 UI，基于 Qt6 QPlainTextEdit，支持命令输入、输出显示、历史命令。

**Architecture:** 子类化 QPlainTextEdit 实现终端显示，分离输入处理逻辑到 TerminalInput 类。通过信号槽机制解耦 UI 与命令处理。

**Tech Stack:** Qt6 (Widgets), C++17, CMake

---

## 文件结构

| 文件 | 职责 |
|------|------|
| `terminal/TerminalWidget.h` | 终端主控件头文件 |
| `terminal/TerminalWidget.cpp` | 终端主控件实现 |
| `terminal/TerminalInput.h` | 输入处理头文件 |
| `terminal/TerminalInput.cpp` | 输入处理实现 |
| `CMakeLists.txt` | 修改：添加新源文件 |
| `main.cpp` | 修改：替换按钮为终端窗口 |

---

### Task 1: 创建目录结构和头文件

**Files:**
- Create: `terminal/TerminalWidget.h`
- Create: `terminal/TerminalInput.h`

- [ ] **Step 1.1: 创建 terminal 目录**

```bash
mkdir terminal
```

- [ ] **Step 1.2: 创建 TerminalWidget.h**

```cpp
#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QPlainTextEdit>

class TerminalInput;

class TerminalWidget : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);

    void appendOutput(const QString &text);
    void setPrompt(const QString &prompt);
    QString currentInput() const;
    void clearTerminal();

signals:
    void commandEntered(const QString &command);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void initStyle();
    void showPrompt();
    void handleEnter();
    void ensureCursorInInputArea();

    TerminalInput *m_input;
    QString m_prompt;
    int m_promptLength;
};

#endif // TERMINALWIDGET_H
```

- [ ] **Step 1.3: 创建 TerminalInput.h**

```cpp
#ifndef TERMINALINPUT_H
#define TERMINALINPUT_H

#include <QObject>
#include <QStringList>

class TerminalInput : public QObject {
    Q_OBJECT

public:
    explicit TerminalInput(QObject *parent = nullptr);

    void addHistory(const QString &command);
    QString previousHistory();
    QString nextHistory();
    void resetHistoryIndex();

private:
    QStringList m_history;
    int m_historyIndex;
};

#endif // TERMINALINPUT_H
```

- [ ] **Step 1.4: 验证文件创建**

确认 `terminal/TerminalWidget.h` 和 `terminal/TerminalInput.h` 存在。

---

### Task 2: 实现 TerminalInput

**Files:**
- Create: `terminal/TerminalInput.cpp`

- [ ] **Step 2.1: 创建 TerminalInput.cpp**

```cpp
#include "TerminalInput.h"

TerminalInput::TerminalInput(QObject *parent)
    : QObject(parent), m_historyIndex(-1) {
}

void TerminalInput::addHistory(const QString &command) {
    if (!command.trimmed().isEmpty()) {
        m_history.append(command);
    }
    m_historyIndex = m_history.size();
}

QString TerminalInput::previousHistory() {
    if (m_history.isEmpty()) return QString();
    if (m_historyIndex > 0) {
        m_historyIndex--;
    }
    return m_history.at(m_historyIndex);
}

QString TerminalInput::nextHistory() {
    if (m_history.isEmpty()) return QString();
    if (m_historyIndex < m_history.size() - 1) {
        m_historyIndex++;
        return m_history.at(m_historyIndex);
    } else {
        m_historyIndex = m_history.size();
        return QString();
    }
}

void TerminalInput::resetHistoryIndex() {
    m_historyIndex = m_history.size();
}
```

- [ ] **Step 2.2: 验证编译**

运行 CMake 配置确认无语法错误（此时还不会链接，因为没有完整构建）。

---

### Task 3: 实现 TerminalWidget

**Files:**
- Create: `terminal/TerminalWidget.cpp`

- [ ] **Step 3.1: 创建 TerminalWidget.cpp**

```cpp
#include "TerminalWidget.h"
#include "TerminalInput.h"
#include <QKeyEvent>
#include <QScrollBar>

TerminalWidget::TerminalWidget(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_input(new TerminalInput(this))
    , m_prompt("user@dish:~$ ")
    , m_promptLength(m_prompt.length())
{
    initStyle();
    showPrompt();
}

void TerminalWidget::initStyle() {
    setStyleSheet(
        "QPlainTextEdit {"
        "  background-color: #000000;"
        "  color: #00FF00;"
        "  font-family: 'Consolas', 'Courier New', monospace;"
        "  font-size: 14px;"
        "  border: none;"
        "  padding: 4px;"
        "}"
    );
    setReadOnly(false);
    setWordWrapMode(QTextOption::WrapAnywhere);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void TerminalWidget::showPrompt() {
    moveCursor(QTextCursor::End);
    QTextCharFormat format;
    format.setForeground(QColor("#FFFFFF"));
    format.setFontWeight(QFont::Bold);
    QTextCursor cursor = textCursor();
    cursor.insertText(m_prompt, format);
    setTextCursor(cursor);

    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(QColor("#00FF00"));
    cursor = textCursor();
    cursor.setCharFormat(defaultFormat);
    setTextCursor(cursor);
}

void TerminalWidget::appendOutput(const QString &text) {
    moveCursor(QTextCursor::End);
    QTextCursor cursor = textCursor();
    cursor.insertText(text + "\n");
    setTextCursor(cursor);
    ensureCursorInInputArea();
}

void TerminalWidget::setPrompt(const QString &prompt) {
    m_prompt = prompt;
    m_promptLength = m_prompt.length();
}

QString TerminalWidget::currentInput() const {
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString blockText = cursor.selectedText();
    if (blockText.length() >= m_promptLength) {
        return blockText.mid(m_promptLength);
    }
    return blockText;
}

void TerminalWidget::clearTerminal() {
    QPlainTextEdit::clear();
    showPrompt();
}

void TerminalWidget::handleEnter() {
    QString input = currentInput();
    m_input->addHistory(input);

    moveCursor(QTextCursor::End);
    QTextCursor cursor = textCursor();
    cursor.insertText("\n");
    setTextCursor(cursor);

    emit commandEntered(input);

    showPrompt();
}

void TerminalWidget::ensureCursorInInputArea() {
    moveCursor(QTextCursor::End);
}

void TerminalWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        handleEnter();
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        int blockStart = cursor.position();
        if (textCursor().position() <= blockStart + m_promptLength) {
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Up) {
        QString prev = m_input->previousHistory();
        if (!prev.isEmpty()) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(m_prompt + prev);
            setTextCursor(cursor);
        }
        return;
    }

    if (event->key() == Qt::Key_Down) {
        QString next = m_input->nextHistory();
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(m_prompt + next);
        setTextCursor(cursor);
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        moveCursor(QTextCursor::End);
        QTextCursor cursor = textCursor();
        cursor.insertText("^C\n");
        setTextCursor(cursor);
        showPrompt();
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_L) {
        clearTerminal();
        return;
    }

    ensureCursorInInputArea();
    QPlainTextEdit::keyPressEvent(event);
}

void TerminalWidget::mousePressEvent(QMouseEvent *event) {
    QPlainTextEdit::mousePressEvent(event);
    ensureCursorInInputArea();
}
```

- [ ] **Step 3.2: 验证代码结构**

确认所有方法实现与头文件声明一致。

---

### Task 4: 更新 CMakeLists.txt

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 4.1: 修改 CMakeLists.txt**

将 `add_executable` 行修改为：

```cmake
add_executable(dish
    main.cpp
    terminal/TerminalWidget.cpp
    terminal/TerminalInput.cpp
)
```

- [ ] **Step 4.2: 验证 CMake 配置**

运行 CMake 配置确认无错误：

```bash
cmake -B cmake-build-debug -S . -G "MinGW Makefiles"
```

---

### Task 5: 更新 main.cpp

**Files:**
- Modify: `main.cpp`

- [ ] **Step 5.1: 替换 main.cpp 内容**

```cpp
#include <QApplication>
#include "terminal/TerminalWidget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    TerminalWidget terminal;
    terminal.setWindowTitle("dish - Linux Terminal Simulator");
    terminal.resize(800, 500);
    terminal.show();

    return QApplication::exec();
}
```

- [ ] **Step 5.2: 验证编译**

```bash
cmake --build cmake-build-debug
```

---

### Task 6: 构建并运行验证

**Files:**
- None (build verification only)

- [ ] **Step 6.1: 完整构建**

```bash
cmake --build cmake-build-debug
```

预期：编译成功，无错误。

- [ ] **Step 6.2: 运行程序**

```bash
./cmake-build-debug/dish.exe
```

预期：弹出 800×500 黑色终端窗口，显示绿色 `user@dish:~$` 提示符。

- [ ] **Step 6.3: 功能验证清单**

| 测试项 | 操作 | 预期结果 |
|--------|------|----------|
| 输入文本 | 键盘输入 `hello` | 绿色文字出现在提示符后 |
| 回车执行 | 按 Enter | 输入回显，新提示符出现 |
| Backspace | 删除到提示符前 | 不允许删除提示符 |
| 上键 | 按上箭头 | 显示上一条历史命令 |
| Ctrl+C | 按 Ctrl+C | 显示 `^C`，换行显示新提示符 |
| Ctrl+L | 按 Ctrl+L | 清屏，显示新提示符 |
| 点击任意位置 | 鼠标点击 | 光标自动回到输入行末尾 |

- [ ] **Step 6.4: 提交代码**

```bash
git add terminal/ CMakeLists.txt main.cpp
git commit -feat: implement terminal simulator UI with QPlainTextEdit
```
