/**
 * - 用户与管理员的注册、登录、登出
 * - MD5密码加密存储
 * - 管道分隔文本文件持久化
 * - 自动初始化root用户
 */

#include "userSystem.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

/**
 * @brief 用户系统构造函数
 *
 * 初始化数据目录，加载用户和管理员数据，自动创建默认root用户
 * 数据文件路径：AppDataLocation/data/users.dat, admins.dat
 */
UserSystem::UserSystem(){
    currentUserId = -1;
    currentAdminId = -1;
    
    /*
    windows 下的文件路径
    文件位置: C:\Users\用户名\AppData\Roaming\dish\data
    文件名: users.dat, admins.dat

    linux 下的文件路径
    文件位置: ~/.local/share/dish/data/
    文件名: users.dat, admins.dat
    */
    
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dataDir(appDataPath + "/data");
    
    if(!dataDir.exists()){
        dataDir.mkpath(".");
    }
    
    QString userDataPath = dataDir.absolutePath() + "/users.dat";
    QString adminDataPath = dataDir.absolutePath() + "/admins.dat";
    
    userDataFile = userDataPath.toStdString();
    adminDataFile = adminDataPath.toStdString();
    
    loadUserData();
    loadAdminData();
    initRootUser();
}

/**
 * @brief 生成新的用户ID
 * @return 新的uid，从1000开始递增（0-999保留给系统用户）
 */
int UserSystem::generateUid(){
    if(users.empty()){
        return 1000;
    }
    int maxUid = 999; // 普通用户从 1000 开始，0-999 保留给系统用户
    for(const auto& user : users){
        if(user.getUid() > maxUid){
            maxUid = user.getUid();
        }
    }
    return maxUid + 1;
}

/**
 * @brief 生成新的管理员ID
 * @return 新的aid，从1000开始递增
 */
int UserSystem::generateAid(){
    if(admins.empty()){
        return 1000;
    }
    int maxAid = 0;
    for(const auto& admin : admins){
        if(admin.getId() > maxAid){
            maxAid = admin.getId();
        }
    }
    return maxAid + 1;
}

/**
 * @brief 检查用户名是否已被普通用户使用
 * @param username 用户名
 * @return 是否已存在
 */
bool UserSystem::usernameExists(const std::string& username){
    for(const auto& user : users){
        if(user.getUsername() == username){
            return true;
        }
    }
    return false;
}

/**
 * @brief 检查用户名是否已被管理员使用
 * @param username 用户名
 * @return 是否已存在
 */
bool UserSystem::adminUsernameExists(const std::string& username){
    for(const auto& admin : admins){
        if(admin.getUsername() == username){
            return true;
        }
    }
    return false;
}

/**
 * @brief 注册新用户
 * @param username 用户名（不能为空）
 * @param password 密码（不能为空）
 * @return 是否注册成功
 *
 * 密码MD5加密后存储，自动分配uid，保存到文件
 */
bool UserSystem::registerUser(std::string username, std::string password){
    if(username.empty() || password.empty()){
        return false;
    }
    if(usernameExists(username)){
        return false;
    }
    std::string encryptedPassword = encryptPassword(password);
    int uid = generateUid();
    User newUser(uid, username, encryptedPassword);
    users.push_back(newUser);
    saveUserData();
    return true;
}

/**
 * @brief 用户登录
 * @param username 用户名
 * @param password 密码
 * @return 是否登录成功
 *
 * 将密码加密后与存储的密码比对
 */
bool UserSystem::loginUser(std::string username, std::string password){
    if(username.empty() || password.empty()){
        return false;
    }
    std::string encryptedPassword = encryptPassword(password);
    for(auto& user : users){
        if(user.login(username, encryptedPassword)){
            currentUserId = user.getUid();
            return true;
        }
    }
    return false;
}

/**
 * @brief 用户登出
 * @return 是否登出成功（未登录时返回false）
 */
bool UserSystem::logoutUser(){
    if(currentUserId == -1){
        return false;
    }
    currentUserId = -1;
    return true;
}

/**
 * @brief 删除用户（需要管理员已登录）
 * @param username 要删除的用户名
 * @return 是否删除成功
 *
 * 如果删除的是当前登录用户，自动重置登录状态
 */
bool UserSystem::deleteUser(std::string username){
    if(currentAdminId == -1){
        return false;
    }
    for(auto it = users.begin(); it != users.end(); ++it){
        if(it->getUsername() == username){
            if(it->getUid() == currentUserId){
                currentUserId = -1;
            }
            users.erase(it);
            saveUserData();
            return true;
        }
    }
    return false;
}

/**
 * @brief 修改用户密码
 * @param username 用户名
 * @param oldPassword 旧密码（管理员修改时可为空）
 * @param newPassword 新密码（不能为空）
 * @return 是否修改成功
 *
 * 管理员可直接修改任意用户密码，普通用户需验证旧密码
 */
bool UserSystem::changeUserPassword(std::string username, std::string oldPassword, std::string newPassword){
    if(currentUserId == -1 && currentAdminId == -1){
        return false;
    }
    if(newPassword.empty()){
        return false;
    }
    std::string encryptedOldPassword = encryptPassword(oldPassword);
    std::string encryptedNewPassword = encryptPassword(newPassword);

    for(auto& user : users){
        if(user.getUsername() == username){
            if(currentAdminId != -1 || (user.getUid() == currentUserId && user.getPassword() == encryptedOldPassword)){
                user.setPassword(encryptedNewPassword);
                saveUserData();
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 注册新管理员
 * @param username 用户名
 * @param password 密码
 * @return 是否注册成功
 */
bool UserSystem::registerAdmin(std::string username, std::string password){
    if(username.empty() || password.empty()){
        return false;
    }
    if(adminUsernameExists(username)){
        return false;
    }
    std::string encryptedPassword = encryptPassword(password);

    Admin newAdmin(generateAid(), username, encryptedPassword);
    admins.push_back(newAdmin);
    saveAdminData();
    return true;
}

/**
 * @brief 管理员登录
 * @param username 用户名
 * @param password 密码
 * @return 是否登录成功
 */
bool UserSystem::loginAdmin(std::string username, std::string password){
    if(username.empty() || password.empty()){
        return false;
    }
    std::string encryptedPassword = encryptPassword(password);

    for(const auto& admin : admins){
        if(admin.getUsername() == username &&
           admin.getPassword() == encryptedPassword){
            currentAdminId = admin.getId();
            return true;
        }
    }
    return false;
}

/**
 * @brief 管理员登出
 * @return 是否登出成功
 */
bool UserSystem::logoutAdmin(){
    if(currentAdminId == -1){
        return false;
    }
    currentAdminId = -1;
    return true;
}

/**
 * @brief 删除管理员（需要管理员已登录）
 * @param username 要删除的用户名
 * @return 是否删除成功
 *
 * 不能删除当前登录的管理员自己
 */
bool UserSystem::deleteAdmin(std::string username){
    if(currentAdminId == -1){
        return false;
    }
    for(auto it = admins.begin(); it != admins.end(); ++it){
        if(it->getUsername() == username){
            if(it->getId() == currentAdminId){
                return false;
            }
            admins.erase(it);
            saveAdminData();
            return true;
        }
    }
    return false;
}

/**
 * @brief 保存用户数据到文件
 * @return 是否保存成功
 *
 * 文件格式：uid|username|encrypted_password（每行一个用户）
 */
bool UserSystem::saveUserData(){
    QFile file(QString::fromStdString(userDataFile));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }
    QTextStream out(&file);
    for(const auto& user : users){
        out << user.getUid() << "|"
            << QString::fromStdString(user.getUsername()) << "|"
            << QString::fromStdString(user.getPassword()) << "\n";
    }
    file.close();
    return true;
}

/**
 * @brief 从文件加载用户数据
 * @return 是否加载成功
 *
 * 文件不存在时视为首次运行，返回true
 */
bool UserSystem::loadUserData(){
    QFile file(QString::fromStdString(userDataFile));
    if(!file.exists()){
        return true;
    }
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return false;
    }
    users.clear();
    QTextStream in(&file);
    while(!in.atEnd()){
        QString line = in.readLine();
        QStringList parts = line.split("|");
        if(parts.size() == 3){
            int uid = parts[0].toInt();
            std::string username = parts[1].toStdString();
            std::string password = parts[2].toStdString();
            users.push_back(User(uid, username, password));
        }
    }
    file.close();
    return true;
}

/**
 * @brief 保存管理员数据到文件
 * @return 是否保存成功
 */
bool UserSystem::saveAdminData(){
    QFile file(QString::fromStdString(adminDataFile));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return false;
    }
    QTextStream out(&file);
    for(const auto& admin : admins){
        out << admin.getId() << "|"
            << QString::fromStdString(admin.getUsername()) << "|"
            << QString::fromStdString(admin.getPassword()) << "\n";
    }
    file.close();
    return true;
}

/**
 * @brief 从文件加载管理员数据
 * @return 是否加载成功
 */
bool UserSystem::loadAdminData(){
    QFile file(QString::fromStdString(adminDataFile));
    if(!file.exists()){
        return true;
    }
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return false;
    }
    admins.clear();
    QTextStream in(&file);
    while(!in.atEnd()){
        QString line = in.readLine();
        QStringList parts = line.split("|");
        if(parts.size() == 3){
            int id = parts[0].toInt();
            std::string username = parts[1].toStdString();
            std::string password = parts[2].toStdString();
            admins.push_back(Admin(id, username, password));
        }
    }
    file.close();
    return true;
}

/**
 * @brief 检查是否有用户登录
 */
bool UserSystem::isLoggedIn() const{
    return currentUserId != -1;
}

/**
 * @brief 检查是否有管理员登录
 */
bool UserSystem::isAdminLoggedIn() const{
    return currentAdminId != -1;
}

/**
 * @brief 获取当前登录用户的指针
 * @return User指针，未登录返回nullptr
 */
User* UserSystem::getCurrentUser(){
    for(auto& user : users){
        if(user.getUid() == currentUserId){
            return &user;
        }
    }
    return nullptr;
}

/**
 * @brief 获取当前登录管理员的指针
 * @return Admin指针，未登录返回nullptr
 */
Admin* UserSystem::getCurrentAdmin(){
    for(auto& admin : admins){
        if(admin.getId() == currentAdminId){
            return &admin;
        }
    }
    return nullptr;
}

/**
 * @brief 使用MD5加密密码
 * @param password 明文密码
 * @return 加密后的十六进制字符串
 */
std::string UserSystem::encryptPassword(const std::string &password){
    QByteArray hash = QCryptographicHash::hash(
        QByteArray::fromStdString(password),
        QCryptographicHash::Md5
    );
    return hash.toHex().toStdString();
}

/**
 * @brief 初始化默认root用户
 *
 * 如果root用户不存在，自动创建uid=0、密码"root"的root用户
 */
void UserSystem::initRootUser(){
    if(!usernameExists("root")){
        std::string encryptedPassword = encryptPassword("root");
        User rootUser(0, "root", encryptedPassword);
        users.insert(users.begin(), rootUser);
        saveUserData();
    }
}

/**
 * @brief 根据用户名查找uid
 * @param username 用户名
 * @return uid值，未找到返回-1
 */
int UserSystem::findUidByUsername(const std::string& username) const{
    for(const auto& user : users){
        if(user.getUsername() == username){
            return user.getUid();
        }
    }
    return -1;
}