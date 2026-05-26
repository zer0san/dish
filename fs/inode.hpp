
#include <cstdint>

const int DIRECT_BLOCKS = 10;

struct Inode {
    uint32_t ino;           // inode编号
    uint32_t mode;          // 文件类型和权限
    uint32_t uid;           // 所有者用户ID
    uint32_t size;          // 文件大小（字节）
    uint32_t atime;         // 访问时间戳
    uint32_t mtime;         // 修改时间戳
    uint32_t ctime;         // 创建时间戳
    uint32_t block_count;   // 占用数据块数
    uint32_t direct_blocks[DIRECT_BLOCKS]; // 直接索引块
    
    Inode();
    void init(uint32_t ino, uint32_t mode, uint32_t uid);
};

/*
这里不使用枚举是为了方便使用位运算
*/

// 文件类型
const uint32_t FILE_TYPE_REGULAR = 0x8000;
const uint32_t FILE_TYPE_DIR = 0x4000;
const uint32_t FILE_TYPE_SYMLINK = 0x2000;

// 权限
const uint32_t PERM_READ = 4;
const uint32_t PERM_WRITE = 2;
const uint32_t PERM_EXEC = 1;

