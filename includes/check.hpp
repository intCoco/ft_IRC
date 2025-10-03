#ifndef CHECK_HPP
#define CHECK_HPP

#include <string>
#include <iostream>
#include <cstdlib>

const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string PINK = "\033[35m";
const std::string RESET = "\033[0m";

struct Data
{
    int port;
    std::string password;
};

class Check
{
    public:
        static Data checkAll(int argc, char **argv);
};

#endif
