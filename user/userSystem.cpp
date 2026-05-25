#include "userSystem.hpp"

UserSystem::UserSystem(){
    currentUserId = -1;
    currentAdminId = -1;
    userDataFile = "users.dat";
    adminDataFile = "admins.dat";
    loadUserData();
    loadAdminData();
}

int UserSystem::generateUid(){
    if(users.empty()){
        return 1000;
    }
    int maxUid = 0;
    for(const auto& user : users){
        if(user.getUid() > maxUid){
            maxUid = user.getUid();
        }
    }
    return maxUid + 1;
}

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

bool UserSystem::usernameExists(const std::string& username){
    for(const auto& user : users){
        if(user.getUsername() == username){
            return true;
        }
    }
    return false;
}

bool UserSystem::adminUsernameExists(const std::string& username){
    for(const auto& admin : admins){
        if(admin.getUsername() == username){
            return true;
        }
    }
    return false;
}

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

bool UserSystem::logoutUser(){
    if(currentUserId == -1){
        return false;
    }
    currentUserId = -1;
    return true;
}

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

bool UserSystem::logoutAdmin(){
    if(currentAdminId == -1){
        return false;
    }
    currentAdminId = -1;
    return true;
}

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

bool UserSystem::isLoggedIn() const{
    return currentUserId != -1;
}

bool UserSystem::isAdminLoggedIn() const{
    return currentAdminId != -1;
}

User* UserSystem::getCurrentUser(){
    for(auto& user : users){
        if(user.getUid() == currentUserId){
            return &user;
        }
    }
    return nullptr;
}

Admin* UserSystem::getCurrentAdmin(){
    for(auto& admin : admins){
        if(admin.getId() == currentAdminId){
            return &admin;
        }
    }
    return nullptr;
}

std::string UserSystem::encryptPassword(const std::string &password){
    QByteArray hash = QCryptographicHash::hash(
        QByteArray::fromStdString(password),
        QCryptographicHash::Md5
    );
    return hash.toHex().toStdString();
}