#include <QCoreApplication>
#include <iostream>
#include "user/userSystem.hpp"

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
    
    std::cout << "测试完成！" << std::endl;
    std::cout << "请检查数据是否已保存到系统应用数据目录下的 dish/data 文件夹中" << std::endl;
}

int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);
    
    testUserSystem();
    
    return 0;
}