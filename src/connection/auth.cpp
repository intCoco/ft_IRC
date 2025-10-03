#include "auth.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

Auth::Auth() : _password("") {}
Auth::Auth(const std::string& password) : _password(password) {}
Auth::Auth(const Auth& other) : _password(other._password) {}
Auth& Auth::operator=(const Auth& other)
{
    if (this != &other)
        _password = other._password;
    return (*this);
}
Auth::~Auth() {}

void Auth::requestPassword(Client* client) const
{
    std::string prompt = "Please enter password: ";
    send(client->getFd(), prompt.c_str(), prompt.size(), 0);
}

bool Auth::verifyPassword(Client* client, const std::string& input) const
{
    std::string trimmed = input;

    if (!trimmed.empty() && trimmed[trimmed.size() - 1] == '\n')
        trimmed.erase(trimmed.size() - 1, 1);
    if (!trimmed.empty() && trimmed[trimmed.size() - 1] == '\r')
        trimmed.erase(trimmed.size() - 1, 1);

    if (trimmed == _password)
    {
        std::cout << "TEST" << std::endl;
        client->setAuthenticated(true);
        std::string ok = "Password correct. Welcome!\r\n";
        send(client->getFd(), ok.c_str(), ok.size(), 0);
        return true;
    }
    else
    {
        std::string fail = "Password incorrect. Try again.\r\n";
        send(client->getFd(), fail.c_str(), fail.size(), 0);
        return false;
    }
}
