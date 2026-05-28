#ifndef INIT_HPP
#define INIT_HPP

#include <QString>

class UserSystem;
class FileSystem;

namespace Init {
    QString initDataPath();
    bool initFileSystem(FileSystem& fileSystem, const QString& diskPath);
    void initStandardDirs(FileSystem& fileSystem);
}

#endif // INIT_HPP