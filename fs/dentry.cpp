#include "dentry.hpp"
#include <cstring>

Dentry::Dentry() : ino(0), name_len(0) {
    memset(name, 0, MAX_FILENAME_LEN);
}

void Dentry::setName(const std::string& filename) {
    name_len = filename.size();
    if (name_len > MAX_FILENAME_LEN) {
        name_len = MAX_FILENAME_LEN;
    }
    memset(name, 0, MAX_FILENAME_LEN);
    strncpy(name, filename.c_str(), name_len);
}

std::string Dentry::getName() const {
    return std::string(name, name_len);
}
