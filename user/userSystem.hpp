#include <string>
#include <vector>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include "./user.hpp"
#include "./admin.hpp"

struct UserSystem{
    std::vector<User> users;
    std::vector<Admin> admins;
    int currentUserId;
    int currentAdminId;
    std::string userDataFile;
    std::string adminDataFile;

    UserSystem();
    bool registerUser(std::string username, std::string password);
    bool loginUser(std::string username, std::string password);
    bool logoutUser();
    bool deleteUser(std::string username);
    bool changeUserPassword(std::string username, std::string oldPassword, std::string newPassword);

    bool registerAdmin(std::string username, std::string password);
    bool loginAdmin(std::string username, std::string password);
    bool logoutAdmin();
    bool deleteAdmin(std::string username);

    bool saveUserData();
    bool loadUserData();
    bool saveAdminData();
    bool loadAdminData();

    bool isLoggedIn() const;
    bool isAdminLoggedIn() const;
    User* getCurrentUser();
    Admin* getCurrentAdmin();

    std::string encryptPassword(const std::string &password);

private:
    int generateUid();
    int generateAid();
    bool usernameExists(const std::string& username);
    bool adminUsernameExists(const std::string& username);
};

