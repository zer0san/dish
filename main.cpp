/*#include <QCoreApplication>
#include <iostream>
#include "user/userSystem.hpp"

void printUserSystemState(UserSystem& system) {
    std::cout << "\n--- 当前状态 ---" << std::endl;
    std::cout << "用户登录状态: " << (system.isLoggedIn() ? "是" : "否") << std::endl;
    std::cout << "管理员登录状态: " << (system.isAdminLoggedIn() ? "是" : "否") << std::endl;
    if (system.getCurrentUser()) {
        std::cout << "当前用户: " << system.getCurrentUser()->getUsername() << std::endl;
    }
    if (system.getCurrentAdmin()) {
        std::cout << "当前管理员: " << system.getCurrentAdmin()->getUsername() << std::endl;
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    std::cout << "=== 第一次运行：注册用户并保存 ===" << std::endl;
    {
        UserSystem system;

        system.registerAdmin("admin", "admin123");
        system.registerUser("user1", "user123");
        system.registerUser("user2", "pass456");

        std::cout << "\n--- 注册完成 ---" << std::endl;
        printUserSystemState(system);

        std::cout << "\n--- 保存数据到文件 ---" << std::endl;
        bool saveUser = system.saveUserData();
        bool saveAdmin = system.saveAdminData();
        std::cout << "保存用户数据: " << (saveUser ? "成功" : "失败") << std::endl;
        std::cout << "保存管理员数据: " << (saveAdmin ? "成功" : "失败") << std::endl;
    }

    std::cout << "\n\n=== 第二次运行：从文件加载数据 ===" << std::endl;
    {
        UserSystem system;

        std::cout << "\n--- 验证登录功能 ---" << std::endl;
        bool loginUser = system.loginUser("user1", "user123");
        std::cout << "使用保存的账号登录 (user1/user123): " << (loginUser ? "成功" : "失败") << std::endl;
        printUserSystemState(system);

        std::cout << "\n--- 测试持久化修改 ---" << std::endl;
        system.changeUserPassword("user1", "user123", "newpassword");
        system.saveUserData();
        std::cout << "修改 user1 密码并保存" << std::endl;

        system.logoutUser();
        bool relogin = system.loginUser("user1", "newpassword");
        std::cout << "使用新密码重新登录: " << (relogin ? "成功" : "失败") << std::endl;
    }

    std::cout << "\n\n=== 第三次运行：验证密码修改持久化 ===" << std::endl;
    {
        UserSystem system;

        std::cout << "\n--- 使用新密码验证登录 ---" << std::endl;
        bool login = system.loginUser("user1", "newpassword");
        std::cout << "user1/newpassword 登录: " << (login ? "成功" : "失败") << std::endl;

        bool wrongLogin = system.loginUser("user1", "user123");
        std::cout << "user1/oldpassword 登录 (应失败): " << (wrongLogin ? "成功" : "失败") << std::endl;
    }

    std::cout << "\n\n=== 测试删除并保存 ===" << std::endl;
    {
        UserSystem system;

        std::cout << "\n--- 删除 user2 ---" << std::endl;
        system.loginAdmin("admin", "admin123");
        bool deleteUser = system.deleteUser("user2");
        std::cout << "删除 user2: " << (deleteUser ? "成功" : "失败") << std::endl;
        system.saveUserData();
    }

    std::cout << "\n\n=== 第四次运行：验证删除持久化 ===" << std::endl;
    {
        UserSystem system;

        std::cout << "\n--- 查看剩余用户 ---" << std::endl;
        printUserSystemState(system);
        std::cout << "user2 应该已被删除" << std::endl;
    }

    std::cout << "\n=== 所有测试完成 ===" << std::endl;

    return 0;
}*/


#include <QApplication>
#include "terminal/TerminalWidget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    TerminalWidget terminal;
    terminal.setWindowTitle("dish - Linux Terminal Simulator");
    terminal.resize(800, 500);
    terminal.show();

    return QApplication::exec();
}