
#pragma once

#include <cstdint>

const int BLOCK_SIZE = 4096;
/*
文件系统魔数
用于判断是否为我们的文件系统
*/
const int MAGIC_NUMBER = 0x12345678; 
// 一个块可以存储的inode数（需要包含inode结构定义）
// const int INODES_PER_BLOCK = BLOCK_SIZE / sizeof(Inode);
// 成组链接法每组块数(需要除去头部)
const int GROUP_ENTRY_COUNT = (BLOCK_SIZE - sizeof(uint32_t)) / sizeof(uint32_t);

struct SuperBlock {
    uint32_t magic;          // 文件系统魔数
    uint32_t block_size;     // 每块大小
    uint32_t total_blocks;   // 总块数
    uint32_t free_blocks;    // 空闲块数
    uint32_t inode_count;    // inode总数
    uint32_t free_inodes;    // 空闲inode数
    uint32_t root_inode;     // 根目录inode号
    uint32_t free_list_head; // 空闲块链表头
    uint32_t group_size;     // 成组链接法每组块数
    uint32_t inode_bitmap_block; // inode位图所在块
    uint32_t data_start_block;   // 数据区起始块
    
    SuperBlock();
    bool isValid() const;
    void init(uint32_t totalBlocks, uint32_t inodeCount);
};
