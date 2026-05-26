#include "superblock.hpp"

SuperBlock::SuperBlock() : magic(0), block_size(BLOCK_SIZE), total_blocks(0), 
    free_blocks(0), inode_count(0), free_inodes(0), root_inode(1), 
    free_list_head(0), group_size(GROUP_ENTRY_COUNT), 
    inode_bitmap_block(1), data_start_block(5) {}  // 与init()保持一致

bool SuperBlock::isValid() const {
    return magic == MAGIC_NUMBER;
}


/*
 * 块号说明：
0：超级块
1：inode位图块
2：inode表（存储inode 1-85）
3：inode表（存储inode 86-100）
4：空闲块链头节点（成组链接法头块）
5：数据块开始块号（根目录在挂载时动态分配）
 */

void SuperBlock::init(uint32_t totalBlocks, uint32_t inodeCount) {
    magic = MAGIC_NUMBER;
    block_size = BLOCK_SIZE;
    total_blocks = totalBlocks;
    inode_count = inodeCount;
    free_inodes = inodeCount - 1;
    root_inode = 1;
    group_size = GROUP_ENTRY_COUNT;
    inode_bitmap_block = 1;
    data_start_block = 5; // 块4用于空闲块组头，数据区从块5开始
    free_list_head = 4; // 空闲块组成组链接法头块存储在块4
    free_blocks = totalBlocks - data_start_block; // 空闲块数 = 块5到末尾
}
