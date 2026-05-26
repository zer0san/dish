#include <QCoreApplication>
#include <iostream>
#include "user/userSystem.hpp"
#include "fs/fs.hpp"

void printSeparator(){
    std::cout << "----------------------------------------" << std::endl;
}

void testUserSystem(){
    std::cout << "开始测试 UserSystem..." << std::endl;
    printSeparator();

    UserSystem userSystem;

    std::cout << "1. 测试用户注册" << std::endl;
    bool regResult1 = userSystem.registerUser("testuser1", "password123");
    std::cout << "   注册 testuser1: " << (regResult1 ? "成功" : "失败") << std::endl;

    bool regResult2 = userSystem.registerUser("testuser2", "password456");
    std::cout << "   注册 testuser2: " << (regResult2 ? "成功" : "失败") << std::endl;

    bool regResult3 = userSystem.registerUser("testuser1", "password789");
    std::cout << "   重复注册 testuser1: " << (regResult3 ? "成功(不应该)" : "失败(预期行为)") << std::endl;
    printSeparator();

    std::cout << "2. 测试用户登录" << std::endl;
    bool loginResult1 = userSystem.loginUser("testuser1", "password123");
    std::cout << "   登录 testuser1: " << (loginResult1 ? "成功" : "失败") << std::endl;

    bool loginResult2 = userSystem.loginUser("testuser1", "wrongpassword");
    std::cout << "   错误密码登录: " << (loginResult2 ? "成功(不应该)" : "失败(预期行为)") << std::endl;

    User* currentUser = userSystem.getCurrentUser();
    if(currentUser){
        std::cout << "   当前登录用户: " << currentUser->getUsername() << std::endl;
    }
    printSeparator();

    std::cout << "3. 测试管理员注册" << std::endl;
    bool adminRegResult1 = userSystem.registerAdmin("admin1", "adminpass123");
    std::cout << "   注册 admin1: " << (adminRegResult1 ? "成功" : "失败") << std::endl;
    printSeparator();

    std::cout << "4. 测试管理员登录" << std::endl;
    bool adminLoginResult = userSystem.loginAdmin("admin1", "adminpass123");
    std::cout << "   登录 admin1: " << (adminLoginResult ? "成功" : "失败") << std::endl;
    printSeparator();

    std::cout << "5. 测试删除用户" << std::endl;
    bool deleteResult = userSystem.deleteUser("testuser2");
    std::cout << "   删除 testuser2: " << (deleteResult ? "成功" : "失败") << std::endl;
    printSeparator();

    std::cout << "6. 测试修改密码" << std::endl;
    bool changePwdResult = userSystem.changeUserPassword("testuser1", "password123", "newpassword123");
    std::cout << "   修改 testuser1 密码: " << (changePwdResult ? "成功" : "失败") << std::endl;

    userSystem.logoutUser();
    bool loginNewPwd = userSystem.loginUser("testuser1", "newpassword123");
    std::cout << "   用新密码登录: " << (loginNewPwd ? "成功" : "失败") << std::endl;
    printSeparator();

    std::cout << "用户系统测试完成！" << std::endl;
}

void testFileSystem() {
    std::cout << "\n开始测试 FileSystem..." << std::endl;
    printSeparator();

    FileSystem fs;
    std::string diskPath = "disk.img";

    std::cout << "1. 格式化磁盘" << std::endl;
    bool formatResult = fs.format(diskPath, 100);
    std::cout << "   格式化结果: " << (formatResult ? "成功" : "失败") << std::endl;
    printSeparator();

    std::cout << "2. 创建目录" << std::endl;
    int homeIno = fs.createDir("/home", 0);
    std::cout << "   创建 /home: " << (homeIno > 0 ? "成功 (ino=" + std::to_string(homeIno) + ")" : "失败") << std::endl;

    int user1Ino = fs.createDir("/home/user1", 1);
    std::cout << "   创建 /home/user1: " << (user1Ino > 0 ? "成功 (ino=" + std::to_string(user1Ino) + ")" : "失败") << std::endl;

    int docsIno = fs.createDir("/home/user1/docs", 1);
    std::cout << "   创建 /home/user1/docs: " << (docsIno > 0 ? "成功 (ino=" + std::to_string(docsIno) + ")" : "失败") << std::endl;
    printSeparator();

    std::cout << "3. 创建文件" << std::endl;
    int file1Ino = fs.createFile("/home/user1/file1.txt", 1);
    std::cout << "   创建 /home/user1/file1.txt: " << (file1Ino > 0 ? "成功 (ino=" + std::to_string(file1Ino) + ")" : "失败") << std::endl;

    int file2Ino = fs.createFile("/home/user1/docs/report.txt", 1);
    std::cout << "   创建 /home/user1/docs/report.txt: " << (file2Ino > 0 ? "成功 (ino=" + std::to_string(file2Ino) + ")" : "失败") << std::endl;
    printSeparator();

    std::cout << "4. 写入文件" << std::endl;
    const char* content = "Hello, File System!";
    int written = fs.writeFile(file1Ino, content, 0, strlen(content));
    std::cout << "   写入 file1.txt: " << written << " 字节" << std::endl;

    const char* reportContent = "课程设计报告\n文件系统实现\n日期: 2024年";
    int written2 = fs.writeFile(file2Ino, reportContent, 0, strlen(reportContent));
    std::cout << "   写入 report.txt: " << written2 << " 字节" << std::endl;
    printSeparator();

    std::cout << "5. 读取文件" << std::endl;
    char buffer[1024] = {0};
    int read = fs.readFile(file1Ino, buffer, 0, 1024);
    std::cout << "   读取 file1.txt: " << read << " 字节" << std::endl;
    std::cout << "   内容: " << buffer << std::endl;
    printSeparator();

    std::cout << "6. 列出目录" << std::endl;
    std::vector<Dentry> entries = fs.listDir("/home/user1");
    std::cout << "   /home/user1 目录内容:" << std::endl;
    for (const auto& entry : entries) {
        std::cout << "     - " << entry.getName() << " (ino=" << entry.ino << ")" << std::endl;
    }
    printSeparator();

    std::cout << "7. 获取文件信息" << std::endl;
    Inode inode = fs.getInode(file1Ino);
    std::cout << "   file1.txt inode信息:" << std::endl;
    std::cout << "     ino: " << inode.ino << std::endl;
    std::cout << "     size: " << inode.size << " 字节" << std::endl;
    std::cout << "     uid: " << inode.uid << std::endl;
    std::cout << "     block_count: " << inode.block_count << std::endl;
    printSeparator();

    std::cout << "8. 权限检查" << std::endl;
    bool canRead = fs.checkPermission(file1Ino, 1, PERM_READ);
    std::cout << "   用户1读取权限: " << (canRead ? "允许" : "拒绝") << std::endl;

    bool canWrite = fs.checkPermission(file1Ino, 1, PERM_WRITE);
    std::cout << "   用户1写入权限: " << (canWrite ? "允许" : "拒绝") << std::endl;

    bool otherRead = fs.checkPermission(file1Ino, 999, PERM_READ);
    std::cout << "   其他用户读取权限: " << (otherRead ? "允许" : "拒绝") << std::endl;
    printSeparator();

    std::cout << "9. 删除文件" << std::endl;
    bool deleteFileResult = fs.deleteFile("/home/user1/file1.txt");
    std::cout << "   删除 file1.txt: " << (deleteFileResult ? "成功" : "失败") << std::endl;
    printSeparator();

    std::cout << "10. 删除空目录" << std::endl;
    bool deleteDirResult = fs.deleteDir("/home/user1/docs");
    std::cout << "   删除 /home/user1/docs: " << (deleteDirResult ? "成功" : "失败") << std::endl;
    printSeparator();

    std::cout << "11. 卸载文件系统" << std::endl;
    fs.unmount();
    std::cout << "   卸载成功" << std::endl;
    printSeparator();

    std::cout << "文件系统测试完成！" << std::endl;
    std::cout << "磁盘文件已保存到: " << diskPath << std::endl;
}

int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);

    // testUserSystem();
    testFileSystem();

    return 0;
}