#include "../../includes/client.hpp"

Client::Client() : _fd(-1), _authenticated(false), _registered(false)
{
    std::cout << YELLOW << "CLIENT: Default constructor" << RESET << std::endl;
}

Client::~Client()
{
    std::cout << YELLOW << "CLIENT: Destructor fd=" << _fd << RESET << std::endl;
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

Client::Client(int fd) : _fd(fd), _authenticated(false), _registered(false)
{
    std::cout << BLUE << "CLIENT: Constructor fd=" << _fd << RESET << std::endl;
}

Client::Client(const Client& copy)
{
    _fd = copy._fd;
    _inputBuffer = copy._inputBuffer;
    _nickname = copy._nickname;
    _username = copy._username;
    _authenticated = copy._authenticated;
    _registered = copy._registered;
}

Client& Client::operator=(const Client& copy)
{
    if (this != &copy)
    {
        _fd = copy._fd;
        _inputBuffer = copy._inputBuffer;
        _nickname = copy._nickname;
        _username = copy._username;
        _authenticated = copy._authenticated;
        _registered = copy._registered;
    }
    return (*this);
}

int Client::getFd() const
{
    return (_fd);
}

const std::string& Client::getNickname() const
{
    return (_nickname);
}

const std::string& Client::getUsername() const
{
    return (_username);
}

bool Client::isAuthenticated() const
{
    return (_authenticated);
}

bool Client::isRegistered() const
{
    return (_registered);
}

void Client::setNickname(const std::string& nick)
{
    _nickname = nick;
    std::cout << GREEN << "CLIENT " << _fd << " nickname = '" 
              << _nickname << "'" << RESET << std::endl;
}

void Client::setUsername(const std::string& user)
{
    _username = user;
    std::cout << GREEN << "CLIENT " << _fd << " username = '" 
              << _username << "'" << RESET << std::endl;
}

void Client::setAuthenticated(bool auth)
{
    _authenticated = auth;
    std::cout << GREEN << "CLIENT " << _fd << " authentification = " 
              << (_authenticated ? "true" : "false") << RESET << std::endl;
}

void Client::setRegistered(bool reg)
{
    _registered = reg;
    std::cout << GREEN << "CLIENT " << _fd << " registred =" 
              << (_registered ? "true" : "false") << RESET << std::endl;
}

void Client::addData(const std::string& data)
{
    _inputBuffer += data;
    std::cout << PINK << "CLIENT " << _fd << " buffer = " << _inputBuffer << RESET << std::endl;
}

std::vector<std::string> Client::getMessage()
{
    std::vector<std::string> messages;
    const std::string delimiter = "\n";                                                                                         // [MODIFICATION] ca va pas "\r\n" car ca cherche les deux d'affilé alors que il peut y avoir simplement \n seul quand le client envoie un message. À RÉADAPTER !
    size_t pos = 0;
    
    std::cout << BLUE << "CLIENT " << _fd << " buffer message = " << RESET << std::endl;
    
    std::string debugBuffer = _inputBuffer;
    for (size_t i = 0; i < debugBuffer.length(); ++i)
    {
        if (debugBuffer[i] == '\r')
            debugBuffer.replace(i, 1, "\\r");
        else if (debugBuffer[i] == '\n')
            debugBuffer.replace(i, 1, "\\n");
    }
    std::cout << BLUE << "CLIENT " << _fd << " current buffer = '" << debugBuffer << "'" << RESET << std::endl;
    
    while ((pos = _inputBuffer.find(delimiter)) != std::string::npos)
    {
        std::string message = _inputBuffer.substr(0, pos);

        if (!message.empty() && message[message.length() - 1] == '\r')
            message.erase(message.length() - 1);
            
        if (!message.empty())
            messages.push_back(message);
        
        _inputBuffer.erase(0, pos + 1);
    }

    std::cout << BLUE << "CLIENT " << _fd << " OK " << messages.size() 
              << " messages. NEXT = '" << _inputBuffer << "'" << RESET << std::endl;
    
    return (messages);
}

bool Client::hasCompleteMessage() const
{
    return (_inputBuffer.find("\r\n") != std::string::npos);
}

void Client::clearBuffer()
{
    _inputBuffer.clear();
    std::cout << YELLOW << "CLIENT " << _fd << "| buffer cleared" << RESET << std::endl;
}

std::string Client::getPrefix() const
{
    if (_nickname.empty())
        return ("");
    
    std::string prefix = _nickname;
    if (!_username.empty())
    {
        prefix += "!" + _username;
        prefix += "@localhost";
    }
    return (prefix);
}