#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QString>
#include <QStringList>
#include <QColor>
#include <QMap>

enum class OutputType {
    Success,
    Error,
    Warning,
    Info
};

struct CommandResult {
    QString output;
    OutputType type;

    static CommandResult success(const QString &text);
    static CommandResult error(const QString &text);
    static CommandResult warning(const QString &text);
    static CommandResult info(const QString &text);

    QColor color() const;
};

struct CommandInfo {
    QString name;
    int minArgs;
    int maxArgs;
    QString usage;
    QString description;
};

class CommandParser {
public:
    CommandParser();

    CommandResult execute(const QString &input);

private:
    void registerCommands();
    CommandResult dispatch(const QString &name, const QStringList &args);

    CommandResult cmdHelp(const QStringList &args);
    CommandResult cmdClear(const QStringList &args);
    CommandResult cmdLogin(const QStringList &args);
    CommandResult cmdLogout(const QStringList &args);
    CommandResult cmdMkdir(const QStringList &args);
    CommandResult cmdRmdir(const QStringList &args);
    CommandResult cmdCd(const QStringList &args);
    CommandResult cmdLs(const QStringList &args);
    CommandResult cmdCreate(const QStringList &args);
    CommandResult cmdDelete(const QStringList &args);
    CommandResult cmdOpen(const QStringList &args);
    CommandResult cmdClose(const QStringList &args);
    CommandResult cmdRead(const QStringList &args);
    CommandResult cmdWrite(const QStringList &args);
    CommandResult cmdSeek(const QStringList &args);
    CommandResult cmdChmod(const QStringList &args);
    CommandResult cmdFormat(const QStringList &args);
    CommandResult cmdStat(const QStringList &args);
    CommandResult cmdPwd(const QStringList &args);
    CommandResult cmdUseradd(const QStringList &args);

    QMap<QString, CommandInfo> m_commands;
};

#endif // COMMANDPARSER_H
