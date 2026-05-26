
#pragma once

#include <cstdint>
#include <string>

// Dentry结构总大小 = uint32_t(4) + uint16_t(2) + char name[250](250) = 256字节
const int MAX_FILENAME_LEN = 250;
const int DENTRY_SIZE = 256;

struct Dentry {
    uint32_t ino;           // 对应的inode编号
    uint16_t name_len;      // 文件名长度
    char name[MAX_FILENAME_LEN]; // 文件名
    
    Dentry();
    void setName(const std::string& name);
    std::string getName() const;
};

