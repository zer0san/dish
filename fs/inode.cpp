#include "inode.hpp"
#include <ctime>

Inode::Inode() : ino(0), mode(0), uid(0), size(0), atime(0), mtime(0), ctime(0), block_count(0) {
    for (int i = 0; i < DIRECT_BLOCKS; ++i) {
        direct_blocks[i] = 0;
    }
}

void Inode::init(uint32_t ino, uint32_t mode, uint32_t uid) {
    this->ino = ino;
    this->mode = mode;
    this->uid = uid;
    this->size = 0;
    this->block_count = 0;
    uint32_t now = time(nullptr);
    this->atime = now;
    this->mtime = now;
    this->ctime = now;
    for (int i = 0; i < DIRECT_BLOCKS; ++i) {
        direct_blocks[i] = 0;
    }
}
