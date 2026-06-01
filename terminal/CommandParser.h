#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QString>
#include <QStringList>
#include <QColor>
#include <QMap>

class UserSystem;
class FileSystem;
struct Inode;

enum class OutputType {
    Success,
    Error,
    Warning,
    Info
};

struct CommandResult {
    QString output;
    OutputType type;
    bool promptChanged;
    QString newPrompt;

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
    CommandParser(UserSystem *userSystem, FileSystem *fileSystem, const QString &diskPath);

    CommandResult execute(const QString &input);

private:
    void registerCommands();
    CommandResult dispatch(const QString &name, const QStringList &args);

    // ===== 路径与权限 =====
    QString resolvePath(const QString &path) const;
    QString normalizePath(const QString &path) const;
    QString buildPrompt() const;
    int getCurrentUid() const;
    bool isRoot() const;

    // 权限检查：root 恒返回 true
    bool canWrite(const QString &path) const;
    bool canRead(const QString &path) const;
    bool canExecute(const QString &path) const;
    bool isOwner(const QString &path) const;

    // 用户家目录管理
    bool createUserHome(const QString &username, int uid);

    // ===== 命令实现 =====
    CommandResult cmdHelp(const QStringList &args);
    CommandResult cmdClear(const QStringList &args);
    CommandResult cmdLogin(const QStringList &args);
    CommandResult cmdLogout(const QStringList &args);
    CommandResult cmdRegister(const QStringList &args);
    CommandResult cmdWhoami(const QStringList &args);
    CommandResult cmdUseradd(const QStringList &args);
    CommandResult cmdMkdir(const QStringList &args);
    CommandResult cmdRmdir(const QStringList &args);
    CommandResult cmdCd(const QStringList &args);
    CommandResult cmdLs(const QStringList &args);
    CommandResult cmdCreate(const QStringList &args);
    CommandResult cmdDelete(const QStringList &args);
    CommandResult cmdRead(const QStringList &args);
    CommandResult cmdWrite(const QStringList &args);
    CommandResult cmdChmod(const QStringList &args);
    CommandResult cmdFormat(const QStringList &args);
    CommandResult cmdStat(const QStringList &args);
    CommandResult cmdPwd(const QStringList &args);
    CommandResult cmdMount(const QStringList &args);
    CommandResult cmdUnmount(const QStringList &args);
    CommandResult cmdMv(const QStringList &args);
    CommandResult cmdMkimg(const QStringList &args);
    CommandResult cmdTree(const QStringList &args);

    // 递归构建目录树
    void buildTree(const QString &path, const QString &prefix, QString &output, bool isLast);

    QMap<QString, CommandInfo> m_commands;
    UserSystem *m_userSystem;
    FileSystem *m_fileSystem;
    QString m_diskPath;
    QString m_currentPath;
    QString m_homePath;
    QString m_lastPath;
};

#endif // COMMANDPARSER_H
