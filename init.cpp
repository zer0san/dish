#include "init.hpp"
#include <QDir>
#include <QStandardPaths>
#include "fs/fs.hpp"

namespace Init {

QString initDataPath() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dataDir(appDataPath + "/data");
    if (!dataDir.exists()) {
        dataDir.mkpath(".");
    }
    return dataDir.absolutePath() + "/disk.img";
}

bool initFileSystem(FileSystem& fileSystem, const QString& diskPath) {
    if (!fileSystem.mount(diskPath.toStdString())) {
        fileSystem.format(diskPath.toStdString(), 100);
        
        int rootIno = fileSystem.createDir("/root", 0);
        if (rootIno > 0) {
            Inode inode = fileSystem.getInode(rootIno);
            inode.mode = FILE_TYPE_DIR | 0700;
            fileSystem.updateInode(rootIno, inode);
        }
        return true;
    }
    return true;
}

void initStandardDirs(FileSystem& fileSystem) {
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
}

} // namespace Init