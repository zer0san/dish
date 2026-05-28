#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include "user/userSystem.hpp"
#include "fs/fs.hpp"
#include "terminal/TerminalWidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 统一数据目录：与 users.dat / admins.dat 同目录
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dataDir(appDataPath + "/data");
    if (!dataDir.exists()) {
        dataDir.mkpath(".");
    }
    QString diskPath = dataDir.absolutePath() + "/disk.img";

    // 1. 初始化用户系统（自动创建 root 用户 uid=0）
    UserSystem userSystem;

    // 2. 初始化文件系统
    FileSystem fileSystem;
    if (!fileSystem.mount(diskPath.toStdString())) {
        // 首次运行，格式化并创建标准目录结构
        fileSystem.format(diskPath.toStdString(), 100);

        // 创建 root 家目录
        int rootIno = fileSystem.createDir("/root", 0);
        if (rootIno > 0) {
            Inode inode = fileSystem.getInode(rootIno);
            inode.mode = FILE_TYPE_DIR | 0700;
            fileSystem.updateInode(rootIno, inode);
        }
    }

    // 3. 补建缺失的标准目录
    if (fileSystem.isMounted()) {
        struct { const char* path; int mode; } dirs[] = {
            {"/home", 0755}, {"/root", 0700},
            {"/tmp",  0777}, {"/etc",  0755},
        };
        for (auto &d : dirs) {
            if (fileSystem.resolvePath(d.path) == -1) {
                int ino = fileSystem.createDir(d.path, 0);
                if (ino > 0) {
                    Inode inode = fileSystem.getInode(ino);
                    inode.mode = FILE_TYPE_DIR | d.mode;
                    fileSystem.updateInode(ino, inode);
                }
            }
        }
    }

    // 4. 启动终端 GUI
    TerminalWidget terminal(&userSystem, &fileSystem, diskPath);
    terminal.setWindowTitle("dish - Linux Terminal Simulator");
    terminal.resize(800, 500);
    terminal.show();

    int ret = QApplication::exec();

    if (fileSystem.isMounted()) {
        fileSystem.unmount();
    }

    return ret;
}
