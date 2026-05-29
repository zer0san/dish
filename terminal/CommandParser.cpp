#include "CommandParser.h"
#include "../user/userSystem.hpp"
#include "../fs/fs.hpp"
#include <cstring>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

// ===== CommandResult =====

CommandResult CommandResult::success(const QString &text) {
    return {text, OutputType::Success, false, ""};
}
CommandResult CommandResult::error(const QString &text) {
    return {text, OutputType::Error, false, ""};
}
CommandResult CommandResult::warning(const QString &text) {
    return {text, OutputType::Warning, false, ""};
}
CommandResult CommandResult::info(const QString &text) {
    return {text, OutputType::Info, false, ""};
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

// ===== 构造 =====

CommandParser::CommandParser(UserSystem *userSystem, FileSystem *fileSystem, const QString &diskPath)
    : m_userSystem(userSystem)
    , m_fileSystem(fileSystem)
    , m_diskPath(diskPath)
    , m_currentPath("/")
    , m_homePath("")
    , m_lastPath("/")
{
    registerCommands();
}

// ===== 路径与权限 =====

QString CommandParser::resolvePath(const QString &path) const {
    if (path.isEmpty() || path == "~") {
        return m_homePath.isEmpty() ? "/" : m_homePath;
    }
    if (path.startsWith("~/")) {
        return (m_homePath.isEmpty() ? "/" : m_homePath) + path.mid(1);
    }
    if (path.startsWith("/")) {
        return path;
    }
    // 相对路径：基于当前目录
    if (m_currentPath == "/") {
        return "/" + path;
    }
    return m_currentPath + "/" + path;
}

QString CommandParser::normalizePath(const QString &path) const {
    QStringList parts;
    for (const auto &p : path.split('/', Qt::SkipEmptyParts)) {
        if (p == "..") {
            if (!parts.isEmpty()) parts.removeLast();
        } else if (p != ".") {
            parts.append(p);
        }
    }
    return "/" + parts.join("/");
}

QString CommandParser::buildPrompt() const {
    QString user = "guest";
    if (m_userSystem->isLoggedIn()) {
        User *u = m_userSystem->getCurrentUser();
        if (u) user = QString::fromStdString(u->getUsername());
    }

    QString displayPath = m_currentPath;
    if (!m_homePath.isEmpty()) {
        if (m_currentPath == m_homePath) {
            displayPath = "~";
        } else if (m_currentPath.startsWith(m_homePath + "/")) {
            displayPath = "~" + m_currentPath.mid(m_homePath.length());
        }
    }
    return user + "@dish:" + displayPath + "$ ";
}

int CommandParser::getCurrentUid() const {
    if (!m_userSystem->isLoggedIn()) return -1;
    User *u = m_userSystem->getCurrentUser();
    return u ? u->getUid() : -1;
}

bool CommandParser::isRoot() const {
    return getCurrentUid() == 0;
}

bool CommandParser::canWrite(const QString &path) const {
    if (!m_fileSystem->isMounted()) return false;
    int uid = getCurrentUid();
    if (uid < 0) return false;
    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) return false;
    return m_fileSystem->checkPermission(ino, uid, PERM_WRITE);
}

bool CommandParser::canRead(const QString &path) const {
    if (!m_fileSystem->isMounted()) return false;
    int uid = getCurrentUid();
    if (uid < 0) return false;
    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) return false;
    return m_fileSystem->checkPermission(ino, uid, PERM_READ);
}

bool CommandParser::canExecute(const QString &path) const {
    if (!m_fileSystem->isMounted()) return false;
    int uid = getCurrentUid();
    if (uid < 0) return false;
    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) return false;
    return m_fileSystem->checkPermission(ino, uid, PERM_EXEC);
}

bool CommandParser::isOwner(const QString &path) const {
    if (!m_fileSystem->isMounted()) return false;
    int uid = getCurrentUid();
    if (uid < 0) return false;
    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) return false;
    Inode inode = m_fileSystem->getInode(ino);
    return uid == (int)inode.uid;
}

bool CommandParser::createUserHome(const QString &username, int uid) {
    if (!m_fileSystem->isMounted()) return false;

    // 确保 /home 存在 (owner=root, mode=755)
    if (m_fileSystem->resolvePath("/home") == -1) {
        int homeIno = m_fileSystem->createDir("/home", 0);
        if (homeIno > 0) {
            Inode inode = m_fileSystem->getInode(homeIno);
            inode.mode = FILE_TYPE_DIR | 0755;
            m_fileSystem->updateInode(homeIno, inode);
        }
    }

    // 创建 /home/<username> (owner=uid, mode=755)
    QString homePath = "/home/" + username;
    if (m_fileSystem->resolvePath(homePath.toStdString()) != -1) return true;

    int ino = m_fileSystem->createDir(homePath.toStdString(), uid);
    if (ino <= 0) return false;

    Inode inode = m_fileSystem->getInode(ino);
    inode.mode = FILE_TYPE_DIR | 0755;
    m_fileSystem->updateInode(ino, inode);
    return true;
}

// ===== 命令注册 =====

void CommandParser::registerCommands() {
    m_commands["help"]     = {"help",     0,  0, "help",                    "显示所有可用命令"};
    m_commands["clear"]    = {"clear",    0,  0, "clear",                   "清屏"};
    m_commands["register"] = {"register", 2,  2, "register <user> <pass>",  "注册新用户并创建家目录"};
    m_commands["login"]    = {"login",    2,  2, "login <user> <pass>",     "用户登录"};
    m_commands["logout"]   = {"logout",   0,  0, "logout",                  "用户登出"};
    m_commands["whoami"]   = {"whoami",   0,  0, "whoami",                  "显示当前用户"};
    m_commands["useradd"]  = {"useradd",  1,  2, "useradd <name> [pass]",   "添加用户（仅root）"};
    m_commands["mkdir"]    = {"mkdir",    1,  1, "mkdir <path>",            "创建目录"};
    m_commands["rmdir"]    = {"rmdir",    1,  1, "rmdir <path>",            "删除空目录"};
    m_commands["cd"]       = {"cd",       0,  1, "cd [path]",               "切换目录（无参数回到家目录）"};
    m_commands["ls"]       = {"ls",       0,  2, "ls [-l] [path]",          "列出目录内容"};
    m_commands["create"]   = {"create",   1,  1, "create <file>",           "创建文件"};
    m_commands["delete"]   = {"delete",   1,  1, "delete <file>",           "删除文件"};
    m_commands["read"]     = {"read",     2,  2, "read <file> <size>",      "读取文件内容"};
    m_commands["write"]    = {"write",    2, -1, "write <file> <text>",     "写入文件"};
    m_commands["chmod"]    = {"chmod",    2,  2, "chmod <file> <mode>",     "修改权限（仅owner/root）"};
    m_commands["format"]   = {"format",   0,  1, "format [blocks]",         "格式化文件系统"};
    m_commands["stat"]     = {"stat",     1,  1, "stat <path>",             "查看文件/目录信息"};
    m_commands["pwd"]      = {"pwd",      0,  0, "pwd",                     "显示当前路径"};
    m_commands["mount"]    = {"mount",    0,  1, "mount [disk]",            "挂载文件系统"};
    m_commands["unmount"]  = {"unmount",  0,  0, "unmount",                 "卸载当前文件系统"};
    m_commands["mv"]       = {"mv",       2,  2, "mv <src> <dst>",          "移动/重命名文件或目录"};
    m_commands["mkimg"]    = {"mkimg",    1,  2, "mkimg <path> [blocks]",   "创建新的磁盘镜像文件"};
}

// ===== 执行与分发 =====

CommandResult CommandParser::execute(const QString &input) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) return CommandResult::success("");

    QStringList parts = trimmed.split(' ', Qt::SkipEmptyParts);
    QString name = parts.first().toLower();
    QStringList args = parts.mid(1);

    if (!m_commands.contains(name)) {
        return CommandResult::error(
            "dish: " + name + ": 未找到命令\n输入 'help' 查看可用命令");
    }

    const CommandInfo &info = m_commands[name];
    if (args.size() < info.minArgs) {
        return CommandResult::error(
            "dish: " + name + ": 参数不足\n用法: " + info.usage);
    }
    if (info.maxArgs >= 0 && args.size() > info.maxArgs) {
        return CommandResult::error(
            "dish: " + name + ": 参数过多\n用法: " + info.usage);
    }

    return dispatch(name, args);
}

CommandResult CommandParser::dispatch(const QString &name, const QStringList &args) {
    if (name == "help")     return cmdHelp(args);
    if (name == "clear")    return cmdClear(args);
    if (name == "register") return cmdRegister(args);
    if (name == "login")    return cmdLogin(args);
    if (name == "logout")   return cmdLogout(args);
    if (name == "whoami")   return cmdWhoami(args);
    if (name == "useradd")  return cmdUseradd(args);
    if (name == "mkdir")    return cmdMkdir(args);
    if (name == "rmdir")    return cmdRmdir(args);
    if (name == "cd")       return cmdCd(args);
    if (name == "ls")       return cmdLs(args);
    if (name == "create")   return cmdCreate(args);
    if (name == "delete")   return cmdDelete(args);
    if (name == "read")     return cmdRead(args);
    if (name == "write")    return cmdWrite(args);
    if (name == "chmod")    return cmdChmod(args);
    if (name == "format")   return cmdFormat(args);
    if (name == "stat")     return cmdStat(args);
    if (name == "pwd")      return cmdPwd(args);
    if (name == "mount")    return cmdMount(args);
    if (name == "unmount")  return cmdUnmount(args);
    if (name == "mv")       return cmdMv(args);
    if (name == "mkimg")    return cmdMkimg(args);
    return CommandResult::error("dish: 内部错误");
}

// ===== 基础命令 =====

CommandResult CommandParser::cmdHelp(const QStringList &) {
    QString output = "可用命令：\n";
    for (auto it = m_commands.constBegin(); it != m_commands.constEnd(); ++it) {
        output += "  " + it->usage.leftJustified(26) + it->description + "\n";
    }
    return CommandResult::info(output.trimmed());
}

CommandResult CommandParser::cmdClear(const QStringList &) {
    return CommandResult::success("\x0C");
}

CommandResult CommandParser::cmdPwd(const QStringList &) {
    return CommandResult::success(m_currentPath);
}

// ===== 用户命令 =====

CommandResult CommandParser::cmdRegister(const QStringList &args) {
    QString username = args[0];
    QString password = args[1];

    if (!m_userSystem->registerUser(username.toStdString(), password.toStdString())) {
        return CommandResult::error("dish: register: 注册失败，用户名 '" + username + "' 已存在");
    }

    int uid = m_userSystem->findUidByUsername(username.toStdString());
    if (uid < 0) {
        return CommandResult::error("dish: register: 注册成功但无法获取 uid");
    }

    if (m_fileSystem->isMounted()) {
        if (!createUserHome(username, uid)) {
            return CommandResult::warning("用户注册成功，但家目录创建失败");
        }
    }

    return CommandResult::success("注册成功: " + username + " (uid=" + QString::number(uid) + ")\n"
                                  "家目录: /home/" + username);
}

CommandResult CommandParser::cmdLogin(const QStringList &args) {
    if (m_userSystem->isLoggedIn()) {
        return CommandResult::error("dish: 已有用户登录，请先 logout");
    }
    QString username = args[0];
    QString password = args[1];

    if (!m_userSystem->loginUser(username.toStdString(), password.toStdString())) {
        return CommandResult::error("dish: 登录失败，用户名或密码错误");
    }

    // 设置家目录
    int uid = getCurrentUid();
    if (uid == 0) {
        m_homePath = "/root";
    } else {
        m_homePath = "/home/" + username;
    }

    // 确保家目录存在
    if (m_fileSystem->isMounted() && m_fileSystem->resolvePath(m_homePath.toStdString()) == -1) {
        if (uid == 0) {
            // root 家目录
            int ino = m_fileSystem->createDir("/root", 0);
            if (ino > 0) {
                Inode inode = m_fileSystem->getInode(ino);
                inode.mode = FILE_TYPE_DIR | 0755;
                m_fileSystem->updateInode(ino, inode);
            }
        } else {
            createUserHome(username, uid);
        }
    }

    m_currentPath = m_homePath;
    m_lastPath = "/";

    CommandResult r = CommandResult::success("登录成功: " + username);
    r.promptChanged = true;
    r.newPrompt = buildPrompt();
    return r;
}

CommandResult CommandParser::cmdLogout(const QStringList &) {
    if (!m_userSystem->isLoggedIn()) {
        return CommandResult::error("dish: 当前没有用户登录");
    }
    m_userSystem->logoutUser();
    m_homePath = "";
    m_currentPath = "/";

    CommandResult r = CommandResult::success("已登出");
    r.promptChanged = true;
    r.newPrompt = buildPrompt();
    return r;
}

CommandResult CommandParser::cmdWhoami(const QStringList &) {
    if (!m_userSystem->isLoggedIn()) {
        return CommandResult::info("(未登录)");
    }
    User *u = m_userSystem->getCurrentUser();
    if (u) {
        return CommandResult::info(QString::fromStdString(u->getUsername())
                                   + " (uid=" + QString::number(u->getUid()) + ")");
    }
    return CommandResult::error("dish: 无法获取当前用户");
}

CommandResult CommandParser::cmdUseradd(const QStringList &args) {
    // Linux: useradd 只有 root 可以执行
    if (!isRoot()) {
        return CommandResult::error("dish: useradd: 权限不足，需要 root 用户");
    }

    QString username = args[0];
    QString password = args.size() > 1 ? args[1] : "123456";

    if (!m_userSystem->registerUser(username.toStdString(), password.toStdString())) {
        return CommandResult::error("dish: useradd: 用户 '" + username + "' 已存在");
    }

    int uid = m_userSystem->findUidByUsername(username.toStdString());
    if (uid < 0) {
        return CommandResult::error("dish: useradd: 用户创建失败");
    }

    if (m_fileSystem->isMounted()) {
        createUserHome(username, uid);
    }

    return CommandResult::success("用户 " + username + " 已创建 (uid=" + QString::number(uid) + ", 密码: " + password + ")");
}

// ===== 文件系统命令 =====

CommandResult CommandParser::cmdFormat(const QStringList &args) {
    int blocks = 100;
    if (!args.isEmpty()) {
        blocks = args[0].toInt();
        if (blocks < 30) {
            return CommandResult::error("dish: format: 块数不能少于30");
        }
    }
    if (m_fileSystem->isMounted()) {
        m_fileSystem->unmount();
    }
    if (!m_fileSystem->format(m_diskPath.toStdString(), blocks)) {
        return CommandResult::error("dish: format: 格式化失败");
    }

    // Linux 标准目录结构，全部由 root (uid=0) 拥有，权限 755
    struct { const char* path; int mode; } dirs[] = {
        {"/home",  0755},
        {"/root",  0700},
        {"/tmp",   0777},
        {"/etc",   0755},
        {"/var",   0755},
        {"/usr",   0755},
    };
    for (auto &d : dirs) {
        int ino = m_fileSystem->createDir(d.path, 0);
        if (ino > 0) {
            Inode inode = m_fileSystem->getInode(ino);
            inode.mode = FILE_TYPE_DIR | d.mode;
            m_fileSystem->updateInode(ino, inode);
        }
    }

    m_currentPath = "/";
    m_homePath = "";

    return CommandResult::success("文件系统格式化完成 (" + QString::number(blocks) + " 块)\n"
                                  "已创建标准目录: /home /root /tmp /etc /var /usr");
}

CommandResult CommandParser::cmdMount(const QStringList &args) {
    if (m_fileSystem->isMounted()) {
        return CommandResult::warning("dish: 文件系统已挂载，请先 unmount 再挂载其他镜像");
    }
    QString disk = args.isEmpty() ? m_diskPath : args[0];
    if (m_fileSystem->mount(disk.toStdString())) {
        // 重置当前路径为根目录
        m_currentPath = "/";
        m_homePath = "";
        m_lastPath = "/";
        
        CommandResult r = CommandResult::success("文件系统已挂载: " + disk);
        r.promptChanged = true;
        r.newPrompt = buildPrompt();
        return r;
    }
    return CommandResult::error("dish: mount: 挂载失败，请先 format 或检查镜像文件是否存在");
}

CommandResult CommandParser::cmdUnmount(const QStringList &args) {
    Q_UNUSED(args)
    
    if (!m_fileSystem->isMounted()) {
        return CommandResult::warning("dish: 没有挂载的文件系统");
    }
    
    // 卸载文件系统
    m_fileSystem->unmount();
    
    // 重置路径状态
    m_currentPath = "/";
    m_homePath = "";
    m_lastPath = "/";
    
    CommandResult r = CommandResult::success("文件系统已卸载");
    r.promptChanged = true;
    r.newPrompt = buildPrompt();
    return r;
}

CommandResult CommandParser::cmdMkdir(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载，请先 format 或 mount");
    }
    QString path = normalizePath(resolvePath(args[0]));

    // Linux: 需要对父目录有写权限
    QString parentPath = path.left(path.lastIndexOf('/'));
    if (parentPath.isEmpty()) parentPath = "/";
    if (!canWrite(parentPath)) {
        return CommandResult::error("dish: mkdir: 无法创建目录 '" + args[0] + "': 权限不足");
    }

    int uid = getCurrentUid();
    if (uid < 0) uid = 0;
    int ino = m_fileSystem->createDir(path.toStdString(), uid);
    if (ino > 0) {
        // 设置目录默认权限：owner 755
        Inode inode = m_fileSystem->getInode(ino);
        inode.mode = FILE_TYPE_DIR | 0755;
        m_fileSystem->updateInode(ino, inode);
        return CommandResult::success("目录已创建: " + path);
    }
    return CommandResult::error("dish: mkdir: 创建失败: " + path);
}

CommandResult CommandParser::cmdRmdir(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = normalizePath(resolvePath(args[0]));

    // Linux: 需要对父目录有写权限
    QString parentPath = path.left(path.lastIndexOf('/'));
    if (parentPath.isEmpty()) parentPath = "/";
    if (!canWrite(parentPath)) {
        return CommandResult::error("dish: rmdir: 权限不足");
    }

    if (m_fileSystem->deleteDir(path.toStdString())) {
        return CommandResult::success("目录已删除: " + path);
    }
    return CommandResult::error("dish: rmdir: 删除失败，目录可能不为空或不存在");
}

CommandResult CommandParser::cmdCd(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("cd: 文件系统未挂载，请先 format 或 mount");
    }

    QString target;
    QString displayArg;  // 用于错误信息显示

    if (args.isEmpty()) {
        // cd 无参数 → 回到家目录（Linux 行为）
        target = m_homePath.isEmpty() ? "/" : m_homePath;
        displayArg = "~";
    } else {
        displayArg = args[0];

        if (displayArg == "-") {
            target = m_lastPath;
        } else if (displayArg == "~") {
            target = m_homePath.isEmpty() ? "/" : m_homePath;
        } else if (displayArg.startsWith("~/")) {
            target = (m_homePath.isEmpty() ? "/" : m_homePath) + displayArg.mid(1);
        } else {
            target = resolvePath(displayArg);
        }
    }

    // 规范化路径（处理 .. 和 .）
    target = normalizePath(target);

    // 检查路径是否存在
    int ino = m_fileSystem->resolvePath(target.toStdString());
    if (ino == -1) {
        return CommandResult::error("cd: " + displayArg + ": 没有那个文件或目录");
    }

    // 检查是否是目录
    Inode inode = m_fileSystem->getInode(ino);
    if (!(inode.mode & FILE_TYPE_DIR)) {
        return CommandResult::error("cd: " + displayArg + ": 不是目录");
    }

    // Linux: 进入目录需要执行权限
    if (!canExecute(target)) {
        return CommandResult::error("cd: " + displayArg + ": 权限不够");
    }

    m_lastPath = m_currentPath;
    m_currentPath = target;

    // cd - 时打印目标路径（Linux 行为）
    QString output;
    if (!args.isEmpty() && args[0] == "-") {
        output = m_currentPath;
    }

    CommandResult r = output.isEmpty()
        ? CommandResult::success("")
        : CommandResult::success(output);
    r.promptChanged = true;
    r.newPrompt = buildPrompt();
    return r;
}

CommandResult CommandParser::cmdLs(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = m_currentPath;
    bool longFormat = false;
    for (const auto &arg : args) {
        if (arg == "-l") longFormat = true;
        else path = resolvePath(arg);
    }
    path = normalizePath(path);

    // Linux: 需要读权限和执行权限
    if (!canRead(path) || !canExecute(path)) {
        return CommandResult::error("dish: ls: 无法访问 '" + path + "': 权限不足");
    }

    std::vector<Dentry> entries = m_fileSystem->listDir(path.toStdString());
    if (entries.empty()) {
        return CommandResult::info("(空目录)");
    }

    QString output;
    for (const auto &entry : entries) {
        QString name = QString::fromStdString(entry.getName());
        Inode inode = m_fileSystem->getInode(entry.ino);
        if (longFormat) {
            QString type = (inode.mode & FILE_TYPE_DIR) ? "d" : "-";
            int p = inode.mode & 0x1FF;
            QString perms;
            perms += (p & 0400) ? "r" : "-";
            perms += (p & 0200) ? "w" : "-";
            perms += (p & 0100) ? "x" : "-";
            perms += (p & 040)  ? "r" : "-";
            perms += (p & 020)  ? "w" : "-";
            perms += (p & 010)  ? "x" : "-";
            perms += (p & 04)   ? "r" : "-";
            perms += (p & 02)   ? "w" : "-";
            perms += (p & 01)   ? "x" : "-";
            output += type + perms + " "
                    + QString::number(inode.uid).rightJustified(4) + " "
                    + QString::number(inode.size).rightJustified(8) + " "
                    + name + "\n";
        } else {
            if (name == "." || name == "..") continue;
            if (inode.mode & FILE_TYPE_DIR) output += name + "/  ";
            else output += name + "  ";
        }
    }
    if (!longFormat) output += "\n";
    return CommandResult::info(output.trimmed());
}

CommandResult CommandParser::cmdCreate(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = normalizePath(resolvePath(args[0]));

    // Linux: 对父目录需要写权限和执行权限
    QString parentPath = path.left(path.lastIndexOf('/'));
    if (parentPath.isEmpty()) parentPath = "/";
    if (!canWrite(parentPath) || !canExecute(parentPath)) {
        return CommandResult::error("dish: create: 权限不足: " + args[0]);
    }

    int uid = getCurrentUid();
    if (uid < 0) uid = 0;
    int ino = m_fileSystem->createFile(path.toStdString(), uid);
    if (ino > 0) {
        return CommandResult::success("文件已创建: " + path + " (ino=" + QString::number(ino) + ")");
    }
    return CommandResult::error("dish: create: 创建失败: " + path);
}

CommandResult CommandParser::cmdDelete(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = normalizePath(resolvePath(args[0]));

    // Linux: 对父目录需要写权限和执行权限
    QString parentPath = path.left(path.lastIndexOf('/'));
    if (parentPath.isEmpty()) parentPath = "/";
    if (!canWrite(parentPath) || !canExecute(parentPath)) {
        return CommandResult::error("dish: delete: 权限不足: " + args[0]);
    }

    if (m_fileSystem->deleteFile(path.toStdString())) {
        return CommandResult::success("文件已删除: " + path);
    }
    return CommandResult::error("dish: delete: 删除失败: " + path);
}

CommandResult CommandParser::cmdRead(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = normalizePath(resolvePath(args[0]));
    int size = args[1].toInt();

    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) {
        return CommandResult::error("dish: read: " + path + ": 没有那个文件");
    }

    // Linux: 需要读权限
    if (!canRead(path)) {
        return CommandResult::error("dish: read: 权限不足: " + path);
    }

    char *buf = new char[size + 1];
    memset(buf, 0, size + 1);
    int bytesRead = m_fileSystem->readFile(ino, buf, 0, size);
    if (bytesRead < 0) {
        delete[] buf;
        return CommandResult::error("dish: read: 读取失败");
    }
    QString content = QString::fromUtf8(buf, bytesRead);
    delete[] buf;
    return CommandResult::info(content);
}

CommandResult CommandParser::cmdWrite(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = normalizePath(resolvePath(args[0]));
    QString text = args.mid(1).join(" ");

    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) {
        return CommandResult::error("dish: write: " + path + ": 没有那个文件");
    }

    // Linux: 需要写权限
    if (!canWrite(path)) {
        return CommandResult::error("dish: write: 权限不足: " + path);
    }

    Inode inode = m_fileSystem->getInode(ino);
    int written = m_fileSystem->writeFile(ino, text.toUtf8().constData(), inode.size, text.toUtf8().size());
    if (written < 0) {
        return CommandResult::error("dish: write: 写入失败");
    }
    return CommandResult::success("写入 " + QString::number(written) + " 字节到 " + path);
}

CommandResult CommandParser::cmdChmod(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = normalizePath(resolvePath(args[0]));

    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) {
        return CommandResult::error("dish: chmod: " + path + ": 没有那个文件或目录");
    }

    // Linux: 只有文件 owner 或 root 可以 chmod
    if (!isRoot() && !isOwner(path)) {
        return CommandResult::error("dish: chmod: 权限不足，只有文件所有者或 root 可以修改权限");
    }

    int mode = args[1].toInt(nullptr, 8);
    Inode inode = m_fileSystem->getInode(ino);
    inode.mode = (inode.mode & 0xFE00) | (mode & 0x1FF);
    m_fileSystem->updateInode(ino, inode);
    return CommandResult::success("权限已修改: " + path + " -> " + QString::number(mode, 8));
}

CommandResult CommandParser::cmdStat(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }
    QString path = normalizePath(resolvePath(args[0]));
    int ino = m_fileSystem->resolvePath(path.toStdString());
    if (ino == -1) {
        return CommandResult::error("dish: stat: " + path + ": 没有那个文件或目录");
    }
    Inode inode = m_fileSystem->getInode(ino);
    QString type = (inode.mode & FILE_TYPE_DIR) ? "目录" : "普通文件";
    QString output;
    output += "  文件: " + path + "\n";
    output += "  类型: " + type + "\n";
    output += "  Inode: " + QString::number(inode.ino) + "\n";
    output += "  权限: " + QString::number(inode.mode & 0x1FF, 8) + "\n";
    output += "  所有者UID: " + QString::number(inode.uid) + "\n";
    output += "  大小: " + QString::number(inode.size) + " 字节\n";
    output += "  数据块数: " + QString::number(inode.block_count) + "\n";
    output += "  创建时间: " + QString::number(inode.ctime) + "\n";
    output += "  修改时间: " + QString::number(inode.mtime) + "\n";
    output += "  访问时间: " + QString::number(inode.atime);
    return CommandResult::info(output);
}

CommandResult CommandParser::cmdMv(const QStringList &args) {
    if (!m_fileSystem->isMounted()) {
        return CommandResult::error("dish: 文件系统未挂载");
    }

    QString srcArg = args[0];
    QString dstArg = args[1];

    QString srcPath = normalizePath(resolvePath(srcArg));
    QString dstPath = normalizePath(resolvePath(dstArg));

    // 检查源是否存在
    int srcIno = m_fileSystem->resolvePath(srcPath.toStdString());
    if (srcIno == -1) {
        return CommandResult::error("dish: mv: 无法访问 '" + srcArg + "': 没有那个文件或目录");
    }

    // 检查对源父目录的写权限
    QString srcParentPath = srcPath.left(srcPath.lastIndexOf('/'));
    if (srcParentPath.isEmpty()) srcParentPath = "/";
    if (!canWrite(srcParentPath)) {
        return CommandResult::error("dish: mv: 无法移动 '" + srcArg + "': 权限不足");
    }

    // 检查对目标父目录的写权限
    int dstIno = m_fileSystem->resolvePath(dstPath.toStdString());
    QString checkPath;
    if (dstIno != -1) {
        Inode dstInode = m_fileSystem->getInode(dstIno);
        if (dstInode.mode & FILE_TYPE_DIR) {
            checkPath = dstPath;
        } else {
            return CommandResult::error("dish: mv: 目标 '" + dstArg + "' 已存在且不是目录");
        }
    } else {
        checkPath = dstPath.left(dstPath.lastIndexOf('/'));
        if (checkPath.isEmpty()) checkPath = "/";
    }
    if (!canWrite(checkPath)) {
        return CommandResult::error("dish: mv: 无法移动到 '" + dstArg + "': 权限不足");
    }

    // 执行移动
    if (m_fileSystem->moveEntry(srcPath.toStdString(), dstPath.toStdString())) {
        return CommandResult::success("'" + srcArg + "' -> '" + dstArg + "'");
    }
    return CommandResult::error("dish: mv: 移动失败");
}

CommandResult CommandParser::cmdMkimg(const QStringList &args) {
    QString imgName = args[0];
    int blocks = 100;  // 默认100块
    
    if (args.size() > 1) {
        blocks = args[1].toInt();
        if (blocks < 30) {
            return CommandResult::error("dish: mkimg: 块数不能少于30");
        }
    }
    
    // 自动添加 .img 后缀（如果没有）
    if (!imgName.endsWith(".img", Qt::CaseInsensitive)) {
        imgName += ".img";
    }
    
    // 获取本地数据目录路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString imagesDir = appDataPath + "/images";
    
    // 确保 images 目录存在
    QDir dir(imagesDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 构建完整的镜像文件路径
    QString imgPath = imagesDir + "/" + imgName;
    
    // 检查文件是否已存在
    QFileInfo fileInfo(imgPath);
    if (fileInfo.exists()) {
        return CommandResult::error("dish: mkimg: 文件 '" + imgName + "' 已存在于 " + imagesDir);
    }
    
    // 调用 FileSystem 的静态方法创建镜像
    if (FileSystem::createImage(imgPath.toStdString(), blocks)) {
        qint64 fileSize = static_cast<qint64>(blocks) * 4096;  // 4KB per block
        QString sizeStr;
        if (fileSize >= 1024 * 1024 * 1024) {
            sizeStr = QString::number(fileSize / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
        } else if (fileSize >= 1024 * 1024) {
            sizeStr = QString::number(fileSize / (1024.0 * 1024.0), 'f', 2) + " MB";
        } else {
            sizeStr = QString::number(fileSize / 1024.0, 'f', 2) + " KB";
        }
        
        return CommandResult::success("镜像文件创建成功并保存到本地:\n"
                                      "  文件名: " + imgName + "\n"
                                      "  保存路径: " + imagesDir + "\n"
                                      "  总块数: " + QString::number(blocks) + "\n"
                                      "  块大小: 4096 字节\n"
                                      "  总大小: " + sizeStr + "\n"
                                      "  提示: 使用 'mount " + imgPath + "' 挂载该镜像");
    }
    
    return CommandResult::error("dish: mkimg: 创建镜像文件失败");
}
