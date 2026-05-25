#include "user.hpp"

User::User(int uid, string username, string password){
    this->uid = uid;
    this->username = username;
    this->password = password;
}

bool User::login(string username, string password){
    return (this->username == username && this->password == password);
}

bool User::changePassword(string newPassword){
    if(newPassword.empty()){
        return false;
    }
    this->password = newPassword;
    return true;
}

bool User::updateUser(string username, string password){
    if(username.empty() || password.empty()){
        return false;
    }
    this->username = username;
    this->password = password;
    return true;
}

int User::getUid() const{
    return uid;
}

string User::getUsername() const{
    return username;
}

string User::getPassword() const{
    return password;
}

void User::setPassword(const string &newPassword){
    this->password = newPassword;
}
