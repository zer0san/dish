#include <string>
using namespace std;

class User{
    private:
        int uid;
        string username;
        string password;

    public:
        User(int uid, string username, string password);
        bool login(string username, string password);
        bool changePassword(string newPassword);
        bool updateUser(string username, string password);
        int getUid() const;
        string getUsername() const;
        string getPassword() const;
        void setPassword(const string &newPassword);
};