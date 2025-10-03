#include "../../includes/check.hpp"

//check args nbr
//check le password
//check le port
Data Check::checkAll(int argc, char **argv)
{
    //std::cout << "--- START CHECKALL ---" << std::endl;
    if (argc != 3)
    {
        std::cerr << RED << "ERROR: wrong number of arguments" << RESET << std::endl;
        std::exit(1);
    }

    std::string checkPort = argv[1];
    //std::cout << "Port = " << checkPort << std::endl;
    std::string checkPassword = argv[2];
    //std::cout << "Password = " << checkPassword << std::endl;

    if (checkPassword.empty())
    {
        std::cerr << RED << "ERROR: password empty" << RESET << std::endl;
        std::exit(1);
    }
    
    if (checkPort.empty())
    {
        std::cerr << RED << "ERROR: port is empty" << RESET << std::endl;
        std::exit(1);
    }

    for (size_t i = 0; i < checkPort.length(); ++i)
    {
        ///std::cout << checkPort << std::endl;
        if (!std::isdigit(checkPort[i]))
        {
            std::cerr << RED << "ERROR: port is not a digit" << RESET << std::endl;
            std::exit(1);
        }
    }

    int port;
    port = std::atoi(checkPort.c_str());
    if (port < 1024 || port > 65535)
    {
        std::cerr << RED << "ERROR: port is a wrong number" << RESET << std::endl;
        std::exit(1);
    }

    Data res;
    res.port = port;
    res.password = checkPassword;

    std::cout << "--- END CHECKALL---" << std::endl;
    std::cout << "Port = " << res.port << std::endl;
    std::cout << "Password = " << res.password << std::endl;
    return (res);
}
