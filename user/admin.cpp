#include "admin.hpp"

Admin::Admin(int id, string username, string password){
    this->id = id;
    this->username = username;
    this->password = password;
}

int Admin::getId() const{
    return id;
}

string Admin::getUsername() const{
    return username;
}

string Admin::getPassword() const{
    return password;
}