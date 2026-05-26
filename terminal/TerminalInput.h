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