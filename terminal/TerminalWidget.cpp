/**
 * - Qt终端界面实现
 * - 键盘事件处理（回车、退格、方向键、快捷键）
 * - 命令自动补全
 * - 命令历史导航
 */

#include "TerminalWidget.h"
#include "TerminalInput.h"
#include "CommandParser.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QStringList>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QContextMenuEvent>

/**
 * @brief 终端组件构造函数
 * @param userSystem 用户系统指针
 * @param fileSystem 文件系统指针
 * @param diskPath 默认磁盘镜像路径
 * @param parent 父窗口
 *
 * 初始化终端样式，显示欢迎信息和提示符
 */
TerminalWidget::TerminalWidget(UserSystem *userSystem, FileSystem *fileSystem, const QString &diskPath, QWidget *parent)
    : QPlainTextEdit(parent)
    , m_input(new TerminalInput(this))
    , m_parser(new CommandParser(userSystem, fileSystem, diskPath))
    , m_prompt("guest@dish:~$ ")
    , m_promptLength(m_prompt.length())
    , m_commands({"login", "logout", "register", "whoami", "mkdir", "rmdir", "cd", "ls",
                  "create", "delete", "open", "close", "read", "write",
                  "seek", "chmod", "format", "stat", "pwd", "useradd", "mount", "help"})
{
    initStyle();
    showWelcomeBanner();
    showPrompt();
}

/**
 * @brief 初始化终端样式
 *
 * 黑色背景、绿色文字、等宽字体、无边框
 */
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

/**
 * @brief 在当前位置显示提示符
 *
 * 提示符用白色粗体显示，之后重置为绿色格式供用户输入
 */
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

/**
 * @brief 追加绿色输出文本
 * @param text 输出内容
 *
 * 自动换行并滚动到底部
 */
void TerminalWidget::appendOutput(const QString &text) {
    moveCursor(QTextCursor::End);
    QTextCursor cursor = textCursor();
    
    // 设置绿色格式
    QTextCharFormat format;
    format.setForeground(QColor("#00FF00"));
    cursor.insertText(text + "\n", format);
    setTextCursor(cursor);
    
    // 自动滚动到底部
    QScrollBar *scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    ensureCursorInInputArea();
}

/**
 * @brief 追加指定颜色的输出文本
 * @param text 输出内容
 * @param color 文字颜色
 */
void TerminalWidget::appendColoredOutput(const QString &text, const QColor &color) {
    moveCursor(QTextCursor::End);
    QTextCursor cursor = textCursor();
    QTextCharFormat format;
    format.setForeground(color);
    cursor.insertText(text + "\n", format);
    
    // 重置为默认绿色格式
    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(QColor("#00FF00"));
    cursor = textCursor();
    cursor.setCharFormat(defaultFormat);
    setTextCursor(cursor);
    
    // 自动滚动到底部
    QScrollBar *scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    ensureCursorInInputArea();
}

/**
 * @brief 显示欢迎横幅
 *
 * ASCII art "dish" logo + 版本信息 + 使用提示
 */
void TerminalWidget::showWelcomeBanner() {
    appendColoredOutput("  ____  _     _     _", QColor("#FFFFFF"));
    appendColoredOutput(" |  _ \\(_)___| |__ | |__   ___  _ __", QColor("#FFFFFF"));
    appendColoredOutput(" | | | | / __| '_ \\| '_ \\ / _ \\| '_ \\", QColor("#FFFFFF"));
    appendColoredOutput(" | |_| | \\__ \\ | | | | | | (_) | | | |", QColor("#FFFFFF"));
    appendColoredOutput(" |____/|_|___|_| |_|_|_|\\___/|_| |_|", QColor("#FFFFFF"));
    appendColoredOutput("", QColor("#FFFFFF"));
    appendColoredOutput(" dish - Linux File System Simulator v0.2", QColor("#00FF00"));
    appendColoredOutput(" Type 'help' for available commands.", QColor("#00FF00"));
    appendColoredOutput("", QColor("#00FF00"));
}

/**
 * @brief 更新提示符内容
 * @param prompt 新的提示符字符串
 */
void TerminalWidget::setPrompt(const QString &prompt) {
    m_prompt = prompt;
    m_promptLength = m_prompt.length();
}

/**
 * @brief 获取当前行的用户输入内容
 * @return 去除提示符后的输入文本
 */
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

/**
 * @brief 清屏，重置为默认绿色格式
 */
void TerminalWidget::clearTerminal() {
    QPlainTextEdit::clear();

    // 设置默认绿色格式
    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(QColor("#00FF00"));
    QTextCursor cursor = textCursor();
    cursor.setCharFormat(defaultFormat);
    setTextCursor(cursor);

    // 注意：不在这里调用 showPrompt()，由 handleEnter() 统一处理
}

/**
 * @brief 处理回车键
 *
 * 流程：获取输入 -> 记录历史 -> 执行命令 -> 显示结果 -> 更新提示符 -> 重新显示提示符
 */
void TerminalWidget::handleEnter() {
    QString input = currentInput();
    m_input->addHistory(input);

    moveCursor(QTextCursor::End);
    QTextCursor cursor = textCursor();
    cursor.insertText("\n");
    setTextCursor(cursor);

    CommandResult result = m_parser->execute(input);
    if (result.output == "\x0C") {
        clearTerminal();
    } else if (!result.output.isEmpty()) {
        appendColoredOutput(result.output, result.color());
    }

    // 处理提示符变更（登录/登出/cd）
    if (result.promptChanged) {
        setPrompt(result.newPrompt);
    }

    showPrompt();
    
    // 自动滚动到底部
    QScrollBar *scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

/**
 * @brief 确保光标在输入区域内（提示符之后）
 *
 * 防止用户点击或导航到提示符区域进行编辑
 */
void TerminalWidget::ensureCursorInInputArea() {
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::StartOfBlock);
    int blockStart = cursor.position();
    int inputStart = blockStart + m_promptLength;
    if (pos < inputStart) {
        cursor.setPosition(inputStart);
        setTextCursor(cursor);
    }
}

/**
 * @brief 键盘事件处理
 * @param event 键盘事件
 *
 * 处理的按键：
 * - Enter: 执行命令
 * - Backspace: 删除字符（不越过提示符）
 * - Up/Down: 命令历史导航
 * - Ctrl+C: 取消当前输入
 * - Ctrl+L: 清屏
 * - Ctrl+V: 粘贴
 * - Tab: 命令自动补全
 * - Home/End: 跳转到行首/行尾
 * - Left: 左移光标（不越过提示符）
 * - Delete: 删除字符（不越过提示符）
 */
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

            // 白色粗体提示符
            QTextCharFormat promptFmt;
            promptFmt.setForeground(QColor("#FFFFFF"));
            promptFmt.setFontWeight(QFont::Bold);
            cursor.setCharFormat(promptFmt);
            cursor.insertText(m_prompt);

            // 绿色输入文本
            QTextCharFormat inputFmt;
            inputFmt.setForeground(QColor("#00FF00"));
            cursor.setCharFormat(inputFmt);
            cursor.insertText(prev);

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

        // 白色粗体提示符
        QTextCharFormat promptFmt;
        promptFmt.setForeground(QColor("#FFFFFF"));
        promptFmt.setFontWeight(QFont::Bold);
        cursor.setCharFormat(promptFmt);
        cursor.insertText(m_prompt);

        // 绿色输入文本（可能为空）
        QTextCharFormat inputFmt;
        inputFmt.setForeground(QColor("#00FF00"));
        cursor.setCharFormat(inputFmt);
        cursor.insertText(next);

        setTextCursor(cursor);
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        // 如果有选中文本，执行复制
        QTextCursor cursor = textCursor();
        QTextCharFormat fmt;
        fmt.setForeground(QColor("#FFFFFF"));
        cursor.setCharFormat(fmt);
        cursor.insertText("^C\n");
        setTextCursor(cursor);
        showPrompt();
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_L) {
        clearTerminal();
        showPrompt();
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V) {
        // 粘贴剪贴板内容
        QString clipboardText = QApplication::clipboard()->text();
        if (!clipboardText.isEmpty()) {
            // 移动到末尾并确保在输入区域
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::End);
            setTextCursor(cursor);

            // 检查光标是否在提示符之后
            ensureCursorInInputArea();

            // 设置绿色格式
            QTextCharFormat format;
            format.setForeground(QColor("#00FF00"));
            cursor = textCursor();
            cursor.setCharFormat(format);

            // 插入粘贴的文本
            cursor.insertText(clipboardText);
            setTextCursor(cursor);
        }
        return;
    }

    if (event->key() == Qt::Key_Tab) {
        handleTab();
        return;
    }

    if (event->key() == Qt::Key_Home) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        int inputStart = cursor.position() + m_promptLength;
        cursor.setPosition(inputStart);
        setTextCursor(cursor);
        return;
    }

    if (event->key() == Qt::Key_End) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        return;
    }

    if (event->key() == Qt::Key_Delete) {
        QTextCursor cursor = textCursor();
        int pos = cursor.position();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        int blockStart = cursor.position();
        int inputStart = blockStart + m_promptLength;
        if (pos < inputStart) {
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Left) {
        QTextCursor cursor = textCursor();
        int pos = cursor.position();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        int inputStart = cursor.position() + m_promptLength;
        if (pos <= inputStart) {
            return;
        }
        QPlainTextEdit::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Right) {
        QPlainTextEdit::keyPressEvent(event);
        return;
    }

    ensureCursorInInputArea();
    
    // 确保输入文本为绿色
    QTextCharFormat format;
    format.setForeground(QColor("#00FF00"));
    QTextCursor cursor = textCursor();
    cursor.setCharFormat(format);
    setTextCursor(cursor);
    
    QPlainTextEdit::keyPressEvent(event);
    
    // 输入后自动滚动到光标位置
    ensureCursorVisible();
}

/**
 * @brief 处理Tab键自动补全
 *
 * 唯一匹配时直接补全并追加空格
 * 多个匹配时显示候选列表并保持原输入
 */
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

        QTextCharFormat fmt;
        fmt.setForeground(QColor("#00FF00"));
        cursor.setCharFormat(fmt);
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

        QTextCharFormat fmt;
        fmt.setForeground(QColor("#00FF00"));
        cursor = textCursor();
        cursor.setCharFormat(fmt);
        cursor.insertText(input);
        setTextCursor(cursor);
    }
}

/**
 * @brief 鼠标点击事件处理
 *
 * 确保点击后光标不会移到提示符区域内
 */
void TerminalWidget::mousePressEvent(QMouseEvent *event) {
    QPlainTextEdit::mousePressEvent(event);
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::StartOfBlock);
    int blockStart = cursor.position();
    int inputStart = blockStart + m_promptLength;
    if (pos < inputStart) {
        cursor.setPosition(inputStart);
        setTextCursor(cursor);
    }
}
