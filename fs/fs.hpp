
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "inode.hpp"
#include "dentry.hpp"
#include "superblock.hpp"

class FileSystem {
public:
    FileSystem();
    ~FileSystem();
    
    bool mount(const std::string& diskPath);
    bool format(const std::string& diskPath, int totalBlocks);
    void unmount();
    
    int createFile(const std::string& path, int uid);
    bool deleteFile(const std::string& path);
    int readFile(int ino, char* buffer, int offset, int size);
    int writeFile(int ino, const char* buffer, int offset, int size);
    bool truncateFile(int ino, int newSize);
    
    int createDir(const std::string& path, int uid);
    bool deleteDir(const std::string& path);
    std::vector<Dentry> listDir(const std::string& path);
    
    Inode getInode(int ino);
    bool updateInode(int ino, const Inode& inode);
    
    bool checkPermission(int ino, int uid, int permission);
    
    int resolvePath(const std::string& path);
    bool isMounted() const { return mounted; }
    
private:
    std::fstream diskFile;
    std::string diskPath;
    SuperBlock superBlock;
    bool mounted;
    
    bool readBlock(int blockNum, char* buffer);
    bool writeBlock(int blockNum, const char* buffer);
    
    int allocBlock();
    void freeBlock(int blockNum);
    
    int allocInode();
    void freeInode(int ino);
    
    bool readSuperBlock();
    bool writeSuperBlock();
    
    bool readInode(int ino, Inode& inode);
    bool writeInode(int ino, const Inode& inode);
    
    std::vector<std::string> splitPath(const std::string& path);
    int findDirEntry(int dirIno, const std::string& name);
    bool addDirEntry(int dirIno, const std::string& name, int ino);
    bool removeDirEntry(int dirIno, const std::string& name);
    
    bool initRootDir();
};

