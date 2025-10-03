#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include "check.hpp"

class Client
{
private:
    int _fd;
    std::string _inputBuffer;
    std::string _nickname;
    std::string _username;
    bool _authenticated;
    bool _registered;

public:
    Client();
    ~Client();
    Client(int fd);
    Client(const Client& copy);
    Client& operator=(const Client& copy);
    
    int getFd() const;
    const std::string& getNickname() const;
    const std::string& getUsername() const;
    bool isAuthenticated() const;
    bool isRegistered() const;
    
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setAuthenticated(bool auth);
    void setRegistered(bool reg);
    
    void addData(const std::string& data);
    std::vector<std::string> getMessage();
    bool hasCompleteMessage() const;
    void clearBuffer();
    std::string getPrefix() const;
};

#endif
