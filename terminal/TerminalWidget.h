#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QPlainTextEdit>
#include <QColor>
#include <QStringList>

class TerminalInput;
class CommandParser;
class UserSystem;
class FileSystem;

class TerminalWidget : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit TerminalWidget(UserSystem *userSystem, FileSystem *fileSystem, const QString &diskPath, QWidget *parent = nullptr);

    void appendOutput(const QString &text);
    void appendColoredOutput(const QString &text, const QColor &color);
    void showWelcomeBanner();
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
    void handleTab();
    void ensureCursorInInputArea();

    TerminalInput *m_input;
    CommandParser *m_parser;
    QString m_prompt;
    int m_promptLength;
    QStringList m_commands;
};

#endif // TERMINALWIDGET_H
