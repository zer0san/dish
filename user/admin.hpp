#include <string>
using namespace std;

class Admin{
    private:
        int id;
        string username;
        string password;

    public:
        Admin(int id, string username, string password);
        int getId() const;
        string getUsername() const;
        string getPassword() const;
};
