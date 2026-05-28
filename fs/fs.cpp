/**
 * - 成组链接法空闲块管理
 * - 一级索引文件
 * - 多级目录结构
 * - 权限管理
 */

#include "fs.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include <ctime>

/**
 * @brief 文件系统构造函数
 * 初始化文件系统状态为未挂载
 */
FileSystem::FileSystem() : mounted(false) {}

/**
 * @brief 文件系统析构函数
 * 如果文件系统已挂载，先卸载
 */
FileSystem::~FileSystem() {
    if (mounted) {
        unmount();
    }
}

/**
 * @brief 挂载文件系统
 * @param diskPath 磁盘文件路径
 * @return 是否挂载成功
 * 
 * 打开磁盘文件，读取超级块进行验证
 */
bool FileSystem::mount(const std::string& diskPath) {
    // 如果已挂载，直接返回失败
    if (mounted) {
        return false;
    }
    
    // 以读写二进制模式打开磁盘文件
    diskFile.open(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    if (!diskFile.is_open()) {
        return false;
    }
    
    // 读取并验证超级块
    if (!readSuperBlock()) {
        diskFile.close();
        return false;
    }
    
    // 设置挂载状态
    this->diskPath = diskPath;
    mounted = true;
    return true;
}

/**
 * @brief 格式化磁盘，创建新文件系统
 * @param diskPath 磁盘文件路径
 * @param totalBlocks 总块数
 * @return 是否格式化成功
 * 
 * 执行步骤：
 * 1. 创建指定大小的磁盘文件
 * 2. 初始化超级块
 * 3. 初始化 inode 位图
 * 4. 初始化成组链接法空闲链表
 * 5. 初始化根目录
 */
bool FileSystem::format(const std::string& diskPath, int totalBlocks) {
    // 步骤1: 创建并扩展磁盘文件到指定大小
    std::fstream file(diskPath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    // 使用稀疏文件技巧：定位到最后一块并写入一个字节
    file.seekp((totalBlocks - 1) * BLOCK_SIZE);
    char zero = 0;
    file.write(&zero, 1);
    file.close();
    
    // 步骤2: 以读写模式打开磁盘文件
    std::fstream fs(diskPath, std::ios::in | std::ios::out | std::ios::binary);
    if (!fs.is_open()) {
        return false;
    }
    
    // 步骤3: 初始化超级块并写入第0块
    SuperBlock sb;
    sb.init(totalBlocks, 100);  // 100个inode
    fs.seekp(0);
    // 将超级块写入第0块
    fs.write(reinterpret_cast<const char*>(&sb), sizeof(SuperBlock));
    
    // 步骤4: 初始化inode位图并写入第1块
    // inode编号从1开始，位图第1位对应inode 1
    char inodeBitmap[BLOCK_SIZE] = {0};
    inodeBitmap[0] |= (1 << 1);  // 设置第1位，表示inode 1（根目录）已使用
    sb.free_inodes = 99;  // 同步更新内存中的空闲inode计数
    fs.seekp(BLOCK_SIZE);
    fs.write(inodeBitmap, BLOCK_SIZE);
    
    // 将更新后的超级块重新写入磁盘
    fs.seekp(0);
    fs.write(reinterpret_cast<const char*>(&sb), sizeof(SuperBlock));
    
    // 步骤5: 初始化inode表的第2，3块为0
    char zeroBlock[BLOCK_SIZE] = {0};
    fs.seekp(2 * BLOCK_SIZE);
    fs.write(zeroBlock, BLOCK_SIZE);
    fs.seekp(3 * BLOCK_SIZE);
    fs.write(zeroBlock, BLOCK_SIZE);

    // 步骤6: 初始化成组链接法空闲块链表并写入第4块
    // 块4用于存储空闲块组，数据区从块5开始
    char groupBlock[BLOCK_SIZE] = {0};
    uint32_t* nums = reinterpret_cast<uint32_t*>(groupBlock);
    int count = 0;
    // 将数据区所有块号存入空闲链表（从data_start_block开始，即块5）
    for (int i = sb.data_start_block; i < totalBlocks; ++i) {
        nums[count++] = i;
    }
    nums[GROUP_ENTRY_COUNT] = 0;  // 最后一个位置存储下一组指针，0表示链表结束
    fs.seekp(4 * BLOCK_SIZE);  // 写入块4
    fs.write(groupBlock, BLOCK_SIZE);
    
    fs.close();
    
    // 步骤7: 挂载并初始化根目录
    if (mount(diskPath)) {
        return initRootDir();
    }
    return false;
}

/**
 * @brief 卸载文件系统
 * 
 * 同步超级块到磁盘，关闭文件句柄
 */
void FileSystem::unmount() {
    if (mounted) {
        // 确保超级块写入磁盘
        writeSuperBlock();
        diskFile.close();
        mounted = false;
    }
}

/**
 * @brief 读取指定块的数据
 * @param blockNum 块号
 * @param buffer 输出缓冲区（必须至少BLOCK_SIZE字节）
 * @return 是否读取成功
 */
bool FileSystem::readBlock(int blockNum, char* buffer) {
    if (!mounted) return false;
    // 定位到指定块的起始位置
    diskFile.seekg(blockNum * BLOCK_SIZE);
    // 读取一个块的数据
    diskFile.read(buffer, BLOCK_SIZE);
    return diskFile.good();
}

/**
 * @brief 写入数据到指定块
 * @param blockNum 块号
 * @param buffer 要写入的数据（必须至少BLOCK_SIZE字节）
 * @return 是否写入成功
 */
bool FileSystem::writeBlock(int blockNum, const char* buffer) {
    if (!mounted) return false;
    // 定位到指定块的起始位置
    diskFile.seekp(blockNum * BLOCK_SIZE);
    // 写入一个块的数据
    diskFile.write(buffer, BLOCK_SIZE);
    // 强制刷新到磁盘
    diskFile.flush();
    return diskFile.good();
}

/**
 * @brief 分配一个空闲块（成组链接法）
 * @return 分配到的块号，失败返回-1
 * 
 * 算法流程：
 * 1. 从当前空闲块组中取出一个块号
 * 2. 如果当前组为空，读取下一组作为当前组
 * 3. 更新空闲块链表和空闲块计数
 */
int FileSystem::allocBlock() {
    // 如果没有空闲块，返回失败
    if (superBlock.free_blocks == 0) return -1;

    // 使用迭代方式避免递归调用
    while (true) {
        // 读取当前空闲块组
        char groupBlock[BLOCK_SIZE];
        readBlock(superBlock.free_list_head, groupBlock);

        uint32_t* nums = reinterpret_cast<uint32_t*>(groupBlock);
        uint32_t nextGroup = nums[GROUP_ENTRY_COUNT];  // 最后一个整数存储下一组指针
        
        // 从后向前查找第一个非零块号
        int allocated = -1;
        for (int i = GROUP_ENTRY_COUNT - 1; i >= 0; --i) {
            if (nums[i] != 0) {
                allocated = nums[i];
                nums[i] = 0;  // 标记为已分配
                break;
            }
        }
        
        // 如果当前组为空，切换到下一组继续查找
        if (allocated == -1) {
            if (nextGroup == 0) return -1;  // 没有更多空闲块
            superBlock.free_list_head = nextGroup;
            continue;  // 继续循环
        }
        
        // 写回更新后的组，并减少空闲块计数
        writeBlock(superBlock.free_list_head, groupBlock);
        superBlock.free_blocks--;
        return allocated;
    }
}

/**
 * @brief 释放一个块（成组链接法）
 * @param blockNum 要释放的块号
 * 
 * 算法流程：
 * 1. 将块号添加到当前空闲块组
 * 2. 如果当前组已满，创建新组
 * 3. 更新空闲块链表和空闲块计数
 */
void FileSystem::freeBlock(int blockNum) {
    // 读取当前空闲块组
    char groupBlock[BLOCK_SIZE];
    readBlock(superBlock.free_list_head, groupBlock);

    uint32_t* nums = reinterpret_cast<uint32_t*>(groupBlock);
    
    // 从后往前查找第一个空位置插入（与allocBlock保持一致）
    for (int i = GROUP_ENTRY_COUNT - 1; i >= 0; --i) {
        if (nums[i] == 0) {
            nums[i] = blockNum;
            writeBlock(superBlock.free_list_head, groupBlock);
            superBlock.free_blocks++;
            return;
        }
    }
    
    // 当前组已满，创建新组
    int oldHead = superBlock.free_list_head;
    
    // 将当前组（包含所有块号和下一组指针）写入新释放的块
    writeBlock(blockNum, groupBlock);
    
    // 更新链表头为新释放的块
    superBlock.free_list_head = blockNum;
    
    // 创建新头块：复制旧组所有条目，末尾追加原头块号
    char newHeadBlock[BLOCK_SIZE] = {0};
    uint32_t* newHeadNums = reinterpret_cast<uint32_t*>(newHeadBlock);
    for (int i = 0; i < GROUP_ENTRY_COUNT; ++i) {
        newHeadNums[i] = nums[i];
    }
    newHeadNums[GROUP_ENTRY_COUNT] = oldHead;  // 原头块成为空闲块加入链表
    
    writeBlock(blockNum, newHeadBlock);
    superBlock.free_list_head = blockNum;
    superBlock.free_blocks++;
}

/**
 * @brief 分配一个空闲inode
 * @return 分配到的inode号，失败返回-1
 * 
 * 使用位图法管理inode分配
 */
int FileSystem::allocInode() {
    // 读取inode位图
    char bitmap[BLOCK_SIZE];
    readBlock(superBlock.inode_bitmap_block, bitmap);

    // 遍历位图查找空闲inode（从1开始，0不用）
    for (int i = 1; i < superBlock.inode_count; ++i) {
        int byte = i / 8;   // 计算字节位置
        int bit = i % 8;    // 计算位位置
        // 如果该位为0，表示inode空闲
        if (!(bitmap[byte] & (1 << bit))) {
            bitmap[byte] |= (1 << bit);  // 标记为已使用
            writeBlock(superBlock.inode_bitmap_block, bitmap);
            superBlock.free_inodes--;
            return i;
        }
    }
    return -1;  // 没有空闲inode
}

/**
 * @brief 释放一个inode
 * @param ino 要释放的inode号
 */
void FileSystem::freeInode(int ino) {
    char bitmap[BLOCK_SIZE];
    readBlock(superBlock.inode_bitmap_block, bitmap);

    int byte = ino / 8;
    int bit = ino % 8;
    bitmap[byte] &= ~(1 << bit);  // 清除该位，标记为空闲
    writeBlock(superBlock.inode_bitmap_block, bitmap);
    superBlock.free_inodes++;
}

/**
 * @brief 从磁盘读取超级块
 * @return 是否读取成功且验证通过
 */
bool FileSystem::readSuperBlock() {
    diskFile.seekg(0);
    diskFile.read(reinterpret_cast<char*>(&superBlock), sizeof(SuperBlock));
    return superBlock.isValid();  // 验证魔数
}

/**
 * @brief 将超级块写入磁盘
 * @return 是否写入成功
 */
bool FileSystem::writeSuperBlock() {
    diskFile.seekp(0);
    diskFile.write(reinterpret_cast<const char*>(&superBlock), sizeof(SuperBlock));
    diskFile.flush();
    return true;
}

/**
 * @brief 读取指定inode
 * @param ino inode号
 * @param inode 输出参数，存储读取的inode
 * @return 是否读取成功
 * 
 * inode表从第2块开始，每块可存储约85个inode
 */
bool FileSystem::readInode(int ino, Inode& inode) {
    // 计算inode所在的块号和偏移
    const int INODES_PER_BLOCK = BLOCK_SIZE / sizeof(Inode);  // 约85个inode/块
    int blockNum = 2 + (ino - 1) / INODES_PER_BLOCK;
    int offset = ((ino - 1) % INODES_PER_BLOCK) * sizeof(Inode);

    char block[BLOCK_SIZE];
    if (!readBlock(blockNum, block)) {
        return false;
    }
    
    // 从块中复制inode数据
    memcpy(&inode, block + offset, sizeof(Inode));
    return inode.ino == ino;
}

/**
 * @brief 写入inode到磁盘
 * @param ino inode号
 * @param inode 要写入的inode数据
 * @return 是否写入成功
 */
bool FileSystem::writeInode(int ino, const Inode& inode) {
    const int INODES_PER_BLOCK = BLOCK_SIZE / sizeof(Inode);
    int blockNum = 2 + (ino - 1) / INODES_PER_BLOCK;
    int offset = ((ino - 1) % INODES_PER_BLOCK) * sizeof(Inode);

    char block[BLOCK_SIZE];
    if (!readBlock(blockNum, block)) {
        return false;
    }
    
    // 将inode复制到块中的对应位置
    memcpy(block + offset, &inode, sizeof(Inode));
    return writeBlock(blockNum, block);
}

/**
 * @brief 获取指定inode
 * @param ino inode号
 * @return inode数据
 */
Inode FileSystem::getInode(int ino) {
    Inode inode;
    readInode(ino, inode);
    return inode;
}

/**
 * @brief 更新inode
 * @param ino inode号
 * @param inode 新的inode数据
 * @return 是否更新成功
 */
bool FileSystem::updateInode(int ino, const Inode& inode) {
    return writeInode(ino, inode);
}

/**
 * @brief 分割路径字符串为组件列表
 * @param path 路径字符串（如 "/home/user/file.txt"）
 * @return 路径组件列表（如 ["home", "user", "file.txt"]）
 */
std::vector<std::string> FileSystem::splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    
    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    return parts;
}

/**
 * @brief 解析路径，返回对应的inode号
 * @param path 文件/目录路径
 * @return inode号，失败返回-1
 * 
 * 从根目录开始逐级查找路径中的每个组件
 */
int FileSystem::resolvePath(const std::string& path) {
    // 特殊处理根目录
    if (path == "/") {
        return superBlock.root_inode;
    }
    
    std::vector<std::string> parts = splitPath(path);
    int currentIno = superBlock.root_inode;
    
    // 逐级查找每个路径组件
    for (const std::string& part : parts) {
        currentIno = findDirEntry(currentIno, part);
        if (currentIno == -1) {
            return -1;
        }
    }
    return currentIno;
}

/**
 * @brief 在目录中查找指定名称的条目
 * @param dirIno 目录的inode号
 * @param name 要查找的文件名/目录名
 * @return 对应条目的inode号，失败返回-1
 */
int FileSystem::findDirEntry(int dirIno, const std::string& name) {
    Inode dirInode;
    if (!readInode(dirIno, dirInode)) {
        return -1;
    }

    // 遍历目录的所有数据块
    for (int i = 0; i < DIRECT_BLOCKS && dirInode.direct_blocks[i] != 0; ++i) {
        char block[BLOCK_SIZE];
        readBlock(dirInode.direct_blocks[i], block);
        
        // 遍历块中的所有目录项
        for (int j = 0; j < BLOCK_SIZE / DENTRY_SIZE; ++j) {
            Dentry* dentry = reinterpret_cast<Dentry*>(block + j * DENTRY_SIZE);
            if (dentry->ino != 0 && dentry->getName() == name) {
                return dentry->ino;
            }
        }
    }
    return -1;
}

/**
 * @brief 向目录中添加条目
 * @param dirIno 目录的inode号
 * @param name 文件名/目录名
 * @param ino 对应文件/目录的inode号
 * @return 是否添加成功
 */
bool FileSystem::addDirEntry(int dirIno, const std::string& name, int ino) {
    Inode dirInode;
    if (!readInode(dirIno, dirInode)) {
        return false;
    }

    // 遍历目录的数据块
    for (int i = 0; i < DIRECT_BLOCKS; ++i) {
        // 如果当前块为空，分配新块
        if (dirInode.direct_blocks[i] == 0) {
            int newBlock = allocBlock();
            if (newBlock == -1) return false;
            dirInode.direct_blocks[i] = newBlock;
            dirInode.size += BLOCK_SIZE;
            dirInode.block_count++;
            // 延迟写入inode，只在成功添加目录项后一次性写入
        }
        
        // 读取目录块
        char block[BLOCK_SIZE];
        readBlock(dirInode.direct_blocks[i], block);

        // 查找空目录项位置
        for (int j = 0; j < BLOCK_SIZE / DENTRY_SIZE; ++j) {
            Dentry* dentry = reinterpret_cast<Dentry*>(block + j * DENTRY_SIZE);
            if (dentry->ino == 0) {
                // 填充目录项
                dentry->ino = ino;
                dentry->setName(name);
                writeBlock(dirInode.direct_blocks[i], block);
                
                // 补回被removeDirEntry扣减的目录大小
                dirInode.size += DENTRY_SIZE;
                writeInode(dirIno, dirInode);
                return true;
            }
        }
    }
    return false;  // 目录已满
}

/**
 * @brief 从目录中删除条目
 * @param dirIno 目录的inode号
 * @param name 要删除的文件名/目录名
 * @return 是否删除成功
 */
bool FileSystem::removeDirEntry(int dirIno, const std::string& name) {
    Inode dirInode;
    if (!readInode(dirIno, dirInode)) {
        return false;
    }

    // 遍历目录的数据块
    for (int i = 0; i < DIRECT_BLOCKS && dirInode.direct_blocks[i] != 0; ++i) {
        char block[BLOCK_SIZE];
        readBlock(dirInode.direct_blocks[i], block);

        // 查找并删除指定条目
        for (int j = 0; j < BLOCK_SIZE / DENTRY_SIZE; ++j) {
            Dentry* dentry = reinterpret_cast<Dentry*>(block + j * DENTRY_SIZE);
            if (dentry->ino != 0 && dentry->getName() == name) {
                // 清空目录项
                dentry->ino = 0;
                dentry->name_len = 0;
                memset(dentry->name, 0, MAX_FILENAME_LEN);
                writeBlock(dirInode.direct_blocks[i], block);
                
                // 目录大小是块大小的倍数，删除单个目录项不改变大小
                // 只有释放整个块时才更新大小
                writeInode(dirIno, dirInode);
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 初始化根目录
 * @return 是否初始化成功
 * 
 * 创建根目录的inode和数据块，添加 "." 和 ".." 条目
 */
bool FileSystem::initRootDir() {
    // 创建根目录inode（inode 1）
    Inode rootInode;
    rootInode.init(1, FILE_TYPE_DIR | 0755, 0);  // 权限755
    
    // 分配根目录数据块
    int rootBlock = allocBlock();
    if (rootBlock == -1) return false;
    
    rootInode.direct_blocks[0] = rootBlock;
    rootInode.size = BLOCK_SIZE;
    rootInode.block_count = 1;
    
    // 创建目录内容（包含 . 和 ..）
    char block[BLOCK_SIZE] = {0};
    
    // "." 指向自己
    Dentry* dentry0 = reinterpret_cast<Dentry*>(block);
    dentry0->ino = 1;
    dentry0->setName(".");
    
    // ".." 也指向自己（根目录的父目录是自己）
    Dentry* dentry1 = reinterpret_cast<Dentry*>(block + DENTRY_SIZE);
    dentry1->ino = 1;
    dentry1->setName("..");
    
    // 写入磁盘
    writeBlock(rootBlock, block);
    writeInode(1, rootInode);
    
    return true;
}

/**
 * @brief 创建文件
 * @param path 文件路径
 * @param uid 文件所有者ID
 * @return 文件的inode号，失败返回-1
 */
int FileSystem::createFile(const std::string& path, int uid) {
    // 检查路径是否已存在
    if (resolvePath(path) != -1) {
        return -1;  // 文件已存在
    }

    // 解析路径
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) {
        return -1;
    }
    
    std::string filename = parts.back();
    parts.pop_back();
    
    // 构建父目录路径
    std::string parentPath = "/";
    if (!parts.empty()) {
        parentPath = "/" + parts[0];
        for (size_t i = 1; i < parts.size(); ++i) {
            parentPath += "/" + parts[i];
        }
    }

    // 获取父目录inode
    int parentIno = resolvePath(parentPath);
    if (parentIno == -1) {
        return -1;
    }

    // 分配inode
    int ino = allocInode();
    if (ino == -1) {
        return -1;
    }
    
    // 初始化inode（普通文件，权限644）
    Inode inode;
    inode.init(ino, FILE_TYPE_REGULAR | 0644, uid);
    writeInode(ino, inode);
    
    // 在父目录中添加条目
    if (!addDirEntry(parentIno, filename, ino)) {
        freeInode(ino);
        return -1;
    }
    
    return ino;
}

/**
 * @brief 删除文件
 * @param path 文件路径
 * @return 是否删除成功
 */
bool FileSystem::deleteFile(const std::string& path) {
    // 获取文件inode
    int ino = resolvePath(path);
    if (ino == -1) {
        return false;
    }

    // 检查是否为普通文件
    Inode inode = getInode(ino);
    if (!(inode.mode & FILE_TYPE_REGULAR)) {
        return false;
    }

    // 释放文件占用的所有数据块
    for (int i = 0; i < DIRECT_BLOCKS && inode.direct_blocks[i] != 0; ++i) {
        freeBlock(inode.direct_blocks[i]);
    }

    // 获取父目录路径
    std::vector<std::string> parts = splitPath(path);
    std::string filename = parts.back();
    parts.pop_back();
    
    std::string parentPath = "/";
    if (!parts.empty()) {
        parentPath = "/" + parts[0];
        for (size_t i = 1; i < parts.size(); ++i) {
            parentPath += "/" + parts[i];
        }
    }

    // 删除父目录中的条目并释放inode
    int parentIno = resolvePath(parentPath);
    if (parentIno == -1) {
        return false;
    }

    // 先删除目录项，失败则不释放inode
    if (!removeDirEntry(parentIno, filename)) {
        return false;
    }
    freeInode(ino);
    
    return true;
}

/**
 * @brief 读取文件内容
 * @param ino 文件的inode号
 * @param buffer 输出缓冲区
 * @param offset 读取起始偏移
 * @param size 要读取的字节数
 * @return 实际读取的字节数，失败返回-1
 */
int FileSystem::readFile(int ino, char* buffer, int offset, int size) {
    // 参数有效性检查
    if (buffer == nullptr || offset < 0 || size < 0) {
        return -1;
    }

    Inode inode;
    if (!readInode(ino, inode)) {
        return -1;
    }

    // 如果偏移超过文件大小，返回0
    if (offset >= static_cast<int>(inode.size)) {
        return 0;
    }
    
    int totalRead = 0;
    int remaining = size;
    
    // 按块读取数据
    while (remaining > 0 && offset < static_cast<int>(inode.size)) {
        int blockIndex = offset / BLOCK_SIZE;      // 计算块索引
        int blockOffset = offset % BLOCK_SIZE;     // 计算块内偏移
        
        // 检查块是否存在
        if (blockIndex >= DIRECT_BLOCKS || inode.direct_blocks[blockIndex] == 0) {
            break;
        }
        
        // 读取数据块
        char block[BLOCK_SIZE];
        readBlock(inode.direct_blocks[blockIndex], block);

        // 计算本次读取量
        int toRead = std::min(remaining, BLOCK_SIZE - blockOffset);
        toRead = std::min(toRead, static_cast<int>(inode.size) - offset);
        
        // 复制数据到缓冲区
        memcpy(buffer + totalRead, block + blockOffset, toRead);
        
        // 更新计数器
        totalRead += toRead;
        remaining -= toRead;
        offset += toRead;
    }
    
    // 更新访问时间
    inode.atime = time(nullptr);
    writeInode(ino, inode);
    
    return totalRead;
}

/**
 * @brief 写入文件内容
 * @param ino 文件的inode号
 * @param buffer 要写入的数据
 * @param offset 写入起始偏移
 * @param size 要写入的字节数
 * @return 实际写入的字节数，失败返回-1
 */
int FileSystem::writeFile(int ino, const char* buffer, int offset, int size) {
    // 参数有效性检查
    if (buffer == nullptr || offset < 0 || size < 0) {
        return -1;
    }

    Inode inode;
    if (!readInode(ino, inode)) {
        return -1;
    }

    int totalWritten = 0;
    int remaining = size;
    int currentOffset = offset;
    
    // 按块写入数据
    while (remaining > 0) {
        int blockIndex = currentOffset / BLOCK_SIZE;
        int blockOffset = currentOffset % BLOCK_SIZE;
        
        // 检查是否超出一级索引限制
        if (blockIndex >= DIRECT_BLOCKS) {
            break;
        }
        
        // 如果块不存在，分配新块
        if (inode.direct_blocks[blockIndex] == 0) {
            int newBlock = allocBlock();
            if (newBlock == -1) {
                break;
            }
            inode.direct_blocks[blockIndex] = newBlock;
            inode.block_count++;
        }
        
        // 读取现有块内容（保持未覆盖部分不变）
        char block[BLOCK_SIZE] = {0};
        readBlock(inode.direct_blocks[blockIndex], block);

        // 计算本次写入量
        int toWrite = std::min(remaining, BLOCK_SIZE - blockOffset);
        
        // 复制数据到块中
        memcpy(block + blockOffset, buffer + totalWritten, toWrite);
        writeBlock(inode.direct_blocks[blockIndex], block);
        
        // 更新计数器
        totalWritten += toWrite;
        remaining -= toWrite;
        currentOffset += toWrite;
    }
    
    // 更新文件大小和修改时间
    if (currentOffset > static_cast<int>(inode.size)) {
        inode.size = currentOffset;
    }
    inode.mtime = time(nullptr);
    writeInode(ino, inode);
    
    return totalWritten;
}

/**
 * @brief 截断文件到指定大小
 * @param ino 文件的inode号
 * @param newSize 新的文件大小
 * @return 是否截断成功
 */
bool FileSystem::truncateFile(int ino, int newSize) {
    Inode inode;
    if (!readInode(ino, inode)) {
        return false;
    }

    // 计算新旧大小对应的块数
    int oldBlocks = (inode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int newBlocks = (newSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // 释放多余的块
    for (int i = newBlocks; i < oldBlocks && i < DIRECT_BLOCKS; ++i) {
        if (inode.direct_blocks[i] != 0) {
            freeBlock(inode.direct_blocks[i]);
            inode.direct_blocks[i] = 0;
            inode.block_count--;
        }
    }
    
    // 更新文件大小和修改时间
    inode.size = newSize;
    inode.mtime = time(nullptr);
    writeInode(ino, inode);
    
    return true;
}

/**
 * @brief 创建目录
 * @param path 目录路径
 * @param uid 目录所有者ID
 * @return 目录的inode号，失败返回-1
 */
int FileSystem::createDir(const std::string& path, int uid) {
    // 检查路径是否已存在
    if (resolvePath(path) != -1) {
        return -1;  // 目录已存在
    }

    // 解析路径
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) {
        return -1;
    }
    
    std::string dirname = parts.back();
    parts.pop_back();
    
    // 构建父目录路径
    std::string parentPath = "/";
    if (!parts.empty()) {
        parentPath = "/" + parts[0];
        for (size_t i = 1; i < parts.size(); ++i) {
            parentPath += "/" + parts[i];
        }
    }

    // 获取父目录inode
    int parentIno = resolvePath(parentPath);
    if (parentIno == -1) {
        return -1;
    }

    // 分配inode
    int ino = allocInode();
    if (ino == -1) {
        return -1;
    }
    
    // 初始化目录inode（权限755）
    Inode inode;
    inode.init(ino, FILE_TYPE_DIR | 0755, uid);
    
    // 分配目录数据块
    int dirBlock = allocBlock();
    if (dirBlock == -1) {
        freeInode(ino);
        return -1;
    }
    
    inode.direct_blocks[0] = dirBlock;
    inode.size = BLOCK_SIZE;
    inode.block_count = 1;
    
    // 创建目录内容（. 和 ..）
    char block[BLOCK_SIZE] = {0};
    
    // "." 指向自己
    Dentry* dentry0 = reinterpret_cast<Dentry*>(block);
    dentry0->ino = ino;
    dentry0->setName(".");
    
    // ".." 指向父目录
    Dentry* dentry1 = reinterpret_cast<Dentry*>(block + DENTRY_SIZE);
    dentry1->ino = parentIno;
    dentry1->setName("..");
    
    // 写入磁盘
    writeBlock(dirBlock, block);
    writeInode(ino, inode);
    
    // 在父目录中添加条目
    if (!addDirEntry(parentIno, dirname, ino)) {
        freeBlock(dirBlock);
        freeInode(ino);
        return -1;
    }
    
    return ino;
}

/**
 * @brief 删除目录
 * @param path 目录路径
 * @return 是否删除成功
 * 
 * 注意：只能删除空目录（仅包含 . 和 ..）
 */
bool FileSystem::deleteDir(const std::string& path) {
    // 获取目录inode
    int ino = resolvePath(path);
    if (ino == -1) {
        return false;
    }

    Inode inode = getInode(ino);

    // 检查是否为目录类型
    if (!(inode.mode & FILE_TYPE_DIR)) {
        return false;
    }

    // 第一步：先检查目录是否为空（只允许包含 . 和 ..）
    int entryCount = 0;
    for (int i = 0; i < DIRECT_BLOCKS && inode.direct_blocks[i] != 0; ++i) {
        char block[BLOCK_SIZE];
        readBlock(inode.direct_blocks[i], block);

        // 统计目录项数量
        for (int j = 0; j < BLOCK_SIZE / DENTRY_SIZE; ++j) {
            Dentry* dentry = reinterpret_cast<Dentry*>(block + j * DENTRY_SIZE);
            if (dentry->ino != 0) {
                entryCount++;
            }
        }
        // 注意：这里不释放块，先检查目录是否为空
    }
    
    // 如果目录项超过2个（. 和 ..），不允许删除
    if (entryCount > 2) {
        return false;
    }
    
    // 第二步：确认目录为空后，再释放数据块
    for (int i = 0; i < DIRECT_BLOCKS && inode.direct_blocks[i] != 0; ++i) {
        freeBlock(inode.direct_blocks[i]);
    }
    
    // 获取父目录路径
    std::vector<std::string> parts = splitPath(path);
    std::string dirname = parts.back();
    parts.pop_back();
    
    std::string parentPath = "/";
    if (!parts.empty()) {
        parentPath = "/" + parts[0];
        for (size_t i = 1; i < parts.size(); ++i) {
            parentPath += "/" + parts[i];
        }
    }

    // 删除父目录中的条目并释放inode
    int parentIno = resolvePath(parentPath);
    if (parentIno == -1) {
        return false;
    }

    // 先删除目录项，失败则不释放inode
    if (!removeDirEntry(parentIno, dirname)) {
        return false;
    }
    freeInode(ino);
    
    return true;
}

/**
 * @brief 列出目录内容
 * @param path 目录路径
 * @return 目录项列表
 */
std::vector<Dentry> FileSystem::listDir(const std::string& path) {
    std::vector<Dentry> entries;
    
    // 获取目录inode
    int ino = resolvePath(path);
    if (ino == -1) {
        return entries;
    }
    
    Inode inode = getInode(ino);

    // 遍历所有目录块
    for (int i = 0; i < DIRECT_BLOCKS && inode.direct_blocks[i] != 0; ++i) {
        char block[BLOCK_SIZE];
        readBlock(inode.direct_blocks[i], block);

        // 收集所有非空目录项
        for (int j = 0; j < BLOCK_SIZE / DENTRY_SIZE; ++j) {
            Dentry* dentry = reinterpret_cast<Dentry*>(block + j * DENTRY_SIZE);
            if (dentry->ino != 0) {
                entries.push_back(*dentry);
            }
        }
    }
    
    return entries;
}

/**
 * @brief 检查用户对文件的权限
 * @param ino 文件的inode号
 * @param uid 用户ID
 * @param permission 要检查的权限（PERM_READ/PERM_WRITE/PERM_EXEC）
 * @return 是否具有该权限
 * 
 * 权限检查规则：
 * - root用户（uid=0）具有所有权限
 * - 普通用户根据所有者/其他用户权限位判断
 */
bool FileSystem::checkPermission(int ino, int uid, int permission) {
    Inode inode = getInode(ino);

    // root用户具有所有权限
    if (uid == 0) {
        return true;
    }
    
    // 获取权限位（低9位）
    uint32_t mode = inode.mode & 0x1FF;
    int perm;
    
    // 判断用户类型
    if (uid == inode.uid) {
        // 所有者权限（位8-6）
        perm = (mode >> 6) & 7;
    } else {
        // 其他用户权限（位2-0）
        perm = mode & 7;
    }
    
    // 检查是否包含所需权限
    return (perm & permission) == permission;
}
