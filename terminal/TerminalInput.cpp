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