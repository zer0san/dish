#include "CommandParser.h"

CommandResult CommandResult::success(const QString &text) {
    return {text, OutputType::Success};
}

CommandResult CommandResult::error(const QString &text) {
    return {text, OutputType::Error};
}

CommandResult CommandResult::warning(const QString &text) {
    return {text, OutputType::Warning};
}

CommandResult CommandResult::info(const QString &text) {
    return {text, OutputType::Info};
}

QColor CommandResult::color() const {
    switch (type) {
        case OutputType::Success: return QColor("#00FF00");
        case OutputType::Error:   return QColor("#FF5555");
        case OutputType::Warning: return QColor("#FFFF55");
        case OutputType::Info:    return QColor("#FFFFFF");
    }
    return QColor("#00FF00");
}

CommandParser::CommandParser() {
    registerCommands();
}

void CommandParser::registerCommands() {
    m_commands["help"]   = {"help",   0, 0, "help",                "显示所有可用命令"};
    m_commands["clear"]  = {"clear",  0, 0, "clear",               "清屏"};
    m_commands["login"]  = {"login",  1, 1, "login <user>",        "用户登录"};
    m_commands["logout"] = {"logout", 0, 0, "logout",              "用户登出"};
    m_commands["mkdir"]  = {"mkdir",  1, 1, "mkdir <path>",        "创建目录"};
    m_commands["rmdir"]  = {"rmdir",  1, 1, "rmdir <path>",        "删除空目录"};
    m_commands["cd"]     = {"cd",     1, 1, "cd <path>",           "切换目录"};
    m_commands["ls"]     = {"ls",     0, 1, "ls [-l]",             "列出目录内容"};
    m_commands["create"] = {"create", 1, 1, "create <file>",       "创建文件"};
    m_commands["delete"] = {"delete", 1, 1, "delete <file>",       "删除文件"};
    m_commands["open"]   = {"open",   2, 2, "open <file> <mode>",  "打开文件"};
    m_commands["close"]  = {"close",  1, 1, "close <fd>",          "关闭文件"};
    m_commands["read"]   = {"read",   2, 2, "read <fd> <size>",    "读取文件"};
    m_commands["write"]  = {"write",  2, 2, "write <fd> <text>",   "写入文件"};
    m_commands["seek"]   = {"seek",   2, 2, "seek <fd> <offset>",  "文件指针定位"};
    m_commands["chmod"]  = {"chmod",  2, 2, "chmod <file> <mode>", "修改权限"};
    m_commands["format"] = {"format", 0, 0, "format",              "格式化文件系统"};
    m_commands["stat"]   = {"stat",   1, 1, "stat <path>",         "查看文件/目录信息"};
    m_commands["pwd"]    = {"pwd",    0, 0, "pwd",                 "显示当前路径"};
    m_commands["useradd"]= {"useradd",1, 1, "useradd <name>",      "添加用户（需root）"};
}

CommandResult CommandParser::execute(const QString &input) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        return CommandResult::success("");
    }

    QStringList parts = trimmed.split(' ', Qt::SkipEmptyParts);
    QString name = parts.first().toLower();
    QStringList args = parts.mid(1);

    if (!m_commands.contains(name)) {
        return CommandResult::error(
            "dish: " + name + ": 未找到命令\n"
            "输入 'help' 查看可用命令"
        );
    }

    const CommandInfo &info = m_commands[name];
    if (args.size() < info.minArgs) {
        return CommandResult::error(
            "dish: " + name + ": 参数不足\n"
            "用法: " + info.usage
        );
    }
    if (info.maxArgs >= 0 && args.size() > info.maxArgs) {
        return CommandResult::error(
            "dish: " + name + ": 参数过多\n"
            "用法: " + info.usage
        );
    }

    return dispatch(name, args);
}

CommandResult CommandParser::dispatch(const QString &name, const QStringList &args) {
    if (name == "help")    return cmdHelp(args);
    if (name == "clear")   return cmdClear(args);
    if (name == "login")   return cmdLogin(args);
    if (name == "logout")  return cmdLogout(args);
    if (name == "mkdir")   return cmdMkdir(args);
    if (name == "rmdir")   return cmdRmdir(args);
    if (name == "cd")      return cmdCd(args);
    if (name == "ls")      return cmdLs(args);
    if (name == "create")  return cmdCreate(args);
    if (name == "delete")  return cmdDelete(args);
    if (name == "open")    return cmdOpen(args);
    if (name == "close")   return cmdClose(args);
    if (name == "read")    return cmdRead(args);
    if (name == "write")   return cmdWrite(args);
    if (name == "seek")    return cmdSeek(args);
    if (name == "chmod")   return cmdChmod(args);
    if (name == "format")  return cmdFormat(args);
    if (name == "stat")    return cmdStat(args);
    if (name == "pwd")     return cmdPwd(args);
    if (name == "useradd") return cmdUseradd(args);

    return CommandResult::error("dish: 内部错误");
}

CommandResult CommandParser::cmdHelp(const QStringList &) {
    QString output = "可用命令：\n";
    for (auto it = m_commands.constBegin(); it != m_commands.constEnd(); ++it) {
        output += "  " + it->usage.leftJustified(22) + it->description + "\n";
    }
    return CommandResult::info(output.trimmed());
}

CommandResult CommandParser::cmdClear(const QStringList &) {
    return CommandResult::success("\x0C");
}

CommandResult CommandParser::cmdLogin(const QStringList &args) {
    return CommandResult::info("[stub] 用户 '" + args[0] + "' 登录成功");
}

CommandResult CommandParser::cmdLogout(const QStringList &) {
    return CommandResult::info("[stub] 用户已登出");
}

CommandResult CommandParser::cmdMkdir(const QStringList &args) {
    return CommandResult::info("[stub] 创建目录: " + args[0]);
}

CommandResult CommandParser::cmdRmdir(const QStringList &args) {
    return CommandResult::info("[stub] 删除目录: " + args[0]);
}

CommandResult CommandParser::cmdCd(const QStringList &args) {
    return CommandResult::info("[stub] 切换目录: " + args[0]);
}

CommandResult CommandParser::cmdLs(const QStringList &args) {
    if (args.isEmpty()) {
        return CommandResult::info("[stub] 列出当前目录内容");
    }
    return CommandResult::info("[stub] 列出当前目录内容 (" + args[0] + ")");
}

CommandResult CommandParser::cmdCreate(const QStringList &args) {
    return CommandResult::info("[stub] 创建文件: " + args[0]);
}

CommandResult CommandParser::cmdDelete(const QStringList &args) {
    return CommandResult::info("[stub] 删除文件: " + args[0]);
}

CommandResult CommandParser::cmdOpen(const QStringList &args) {
    return CommandResult::info("[stub] 打开文件: " + args[0] + " 模式: " + args[1]);
}

CommandResult CommandParser::cmdClose(const QStringList &args) {
    return CommandResult::info("[stub] 关闭文件描述符: " + args[0]);
}

CommandResult CommandParser::cmdRead(const QStringList &args) {
    return CommandResult::info("[stub] 读取 fd=" + args[0] + " 大小=" + args[1]);
}

CommandResult CommandParser::cmdWrite(const QStringList &args) {
    return CommandResult::info("[stub] 写入 fd=" + args[0] + " 内容: " + args[1]);
}

CommandResult CommandParser::cmdSeek(const QStringList &args) {
    return CommandResult::info("[stub] 定位 fd=" + args[0] + " 偏移=" + args[1]);
}

CommandResult CommandParser::cmdChmod(const QStringList &args) {
    return CommandResult::info("[stub] 修改权限: " + args[0] + " -> " + args[1]);
}

CommandResult CommandParser::cmdFormat(const QStringList &) {
    return CommandResult::warning("[stub] 格式化文件系统（未实现）");
}

CommandResult CommandParser::cmdStat(const QStringList &args) {
    return CommandResult::info("[stub] 查看信息: " + args[0]);
}

CommandResult CommandParser::cmdPwd(const QStringList &) {
    return CommandResult::success("/");
}

CommandResult CommandParser::cmdUseradd(const QStringList &args) {
    return CommandResult::info("[stub] 添加用户: " + args[0]);
}
