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

TerminalWidget::TerminalWidget(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_input(new TerminalInput(this))
    , m_parser(new CommandParser())
    , m_prompt("user@dish:~$ ")
    , m_promptLength(m_prompt.length())
    , m_commands({"login", "logout", "mkdir", "rmdir", "cd", "ls",
                  "create", "delete", "open", "close", "read", "write",
                  "seek", "chmod", "format", "stat", "pwd", "useradd", "help"})
{
    initStyle();
    showWelcomeBanner();
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
    
    // 设置默认绿色格式
    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(QColor("#00FF00"));
    QTextCursor cursor = textCursor();
    cursor.setCharFormat(defaultFormat);
    setTextCursor(cursor);
    
    showPrompt();
}

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

    // 重置颜色为绿色
    QTextCharFormat defaultFormat;
    defaultFormat.setForeground(QColor("#00FF00"));
    cursor = textCursor();
    cursor.setCharFormat(defaultFormat);
    setTextCursor(cursor);

    showPrompt();
    
    // 自动滚动到底部
    QScrollBar *scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

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
            
            // 设置白色粗体格式插入提示符
            QTextCharFormat promptFormat;
            promptFormat.setForeground(QColor("#FFFFFF"));
            promptFormat.setFontWeight(QFont::Bold);
            cursor.insertText(m_prompt, promptFormat);
            
            // 设置绿色格式插入历史命令
            QTextCharFormat inputFormat;
            inputFormat.setForeground(QColor("#00FF00"));
            cursor.insertText(prev, inputFormat);
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
        
        // 设置白色粗体格式插入提示符
        QTextCharFormat promptFormat;
        promptFormat.setForeground(QColor("#FFFFFF"));
        promptFormat.setFontWeight(QFont::Bold);
        cursor.insertText(m_prompt, promptFormat);
        
        // 设置绿色格式插入历史命令
        QTextCharFormat inputFormat;
        inputFormat.setForeground(QColor("#00FF00"));
        cursor.insertText(next, inputFormat);
        setTextCursor(cursor);
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        // 如果有选中文本，执行复制
        QTextCursor cursor = textCursor();
        if (cursor.hasSelection()) {
            // 复制选中文本到剪贴板
            QApplication::clipboard()->setText(cursor.selectedText());
        } else {
            // 没有选中文本，显示 ^C
            moveCursor(QTextCursor::End);
            cursor = textCursor();
            QTextCharFormat format;
            format.setForeground(QColor("#FFFFFF"));
            cursor.insertText("^C\n", format);
            setTextCursor(cursor);
            showPrompt();
        }
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_L) {
        clearTerminal();
        return;
    }

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V) {
        // 粘贴剪贴板内容
        QString clipboardText = QApplication::clipboard()->text();
        if (!clipboardText.isEmpty()) {
            // 确保光标在输入区域
            ensureCursorInInputArea();
            
            // 设置绿色格式
            QTextCharFormat format;
            format.setForeground(QColor("#00FF00"));
            QTextCursor cursor = textCursor();
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
        
        // 设置绿色格式再插入
        QTextCharFormat format;
        format.setForeground(QColor("#00FF00"));
        cursor.insertText(completion + " ", format);
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
        
        // 设置绿色格式再插入用户输入
        QTextCharFormat format;
        format.setForeground(QColor("#00FF00"));
        cursor = textCursor();
        cursor.insertText(input, format);
        setTextCursor(cursor);
    }
}

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

void TerminalWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    // 双击选择单词
    QPlainTextEdit::mouseDoubleClickEvent(event);
    
    // 确保选择在输入区域内
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        int selectionStart = cursor.selectionStart();
        int selectionEnd = cursor.selectionEnd();
        
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        int blockStart = cursor.position();
        int inputStart = blockStart + m_promptLength;
        
        // 如果选择开始位置在提示符之前，调整到输入区域开始
        if (selectionStart < inputStart) {
            cursor.setPosition(inputStart);
            cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
            setTextCursor(cursor);
        }
    }
}

void TerminalWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    
    // 获取当前光标
    QTextCursor cursor = textCursor();
    
    // 复制动作
    QAction *copyAction = menu.addAction("复制");
    copyAction->setEnabled(cursor.hasSelection());
    connect(copyAction, &QAction::triggered, [this, cursor]() {
        if (cursor.hasSelection()) {
            QApplication::clipboard()->setText(cursor.selectedText());
        }
    });
    
    // 粘贴动作
    QAction *pasteAction = menu.addAction("粘贴");
    pasteAction->setEnabled(!QApplication::clipboard()->text().isEmpty());
    connect(pasteAction, &QAction::triggered, [this]() {
        QString clipboardText = QApplication::clipboard()->text();
        if (!clipboardText.isEmpty()) {
            ensureCursorInInputArea();
            
            QTextCharFormat format;
            format.setForeground(QColor("#00FF00"));
            QTextCursor cursor = textCursor();
            cursor.setCharFormat(format);
            
            cursor.insertText(clipboardText);
            setTextCursor(cursor);
        }
    });
    
    // 全选动作
    QAction *selectAllAction = menu.addAction("全选");
    connect(selectAllAction, &QAction::triggered, [this]() {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock);
        int blockStart = cursor.position();
        int inputStart = blockStart + m_promptLength;
        
        cursor.setPosition(inputStart);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    });
    
    menu.exec(event->globalPos());
}