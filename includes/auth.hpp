#ifndef AUTH_HPP
#define AUTH_HPP

#include "client.hpp"
#include <string>

class Auth
{
private:
    std::string _password;

public:
    Auth();
    Auth(const std::string& password);
    Auth(const Auth& other);
    Auth& operator=(const Auth& other);
    ~Auth();

    void requestPassword(Client* client) const;
    bool verifyPassword(Client* client, const std::string& input) const;
};

#endif
