#include "../../includes/check.hpp"
#include "../../includes/message.hpp"
#include <iostream>

Message::Message()
{
    _command = "";
}

Message::~Message()
{}

Message::Message(const Message& other)
{
    _command = other._command;
    _params = other._params;
}

Message& Message::operator=(const Message& other)
{
    if (this != &other)
    {
        _command = other._command;
        _params = other._params;
    }
    return (*this);
}

//PARSE COMMAND
bool Message::deleteSpace(const std::string& input)
{
    std::cout << "--- PARSING " << input << " ---" << std::endl;
    
    if (input.empty())
    {
        std::cout << RED << "EMPTY INPUT" << RESET << std::endl;
        return (false);
    }
    
    _command = "";
    _params.clear();
    
    std::vector<std::string> tokens;
    size_t pos = 0;
    
    while (pos < input.length())
    {
        while (pos < input.length() && input[pos] == ' ')
            pos++;
            
        if (pos >= input.length())
            break ;
            
        if (input[pos] == ':' && tokens.size() > 0)
        {
            std::string trailing = input.substr(pos + 1);
            tokens.push_back(trailing);
            break ;
        }

        size_t end = input.find(' ', pos);
        if (end == std::string::npos)
            end = input.length();
            
        tokens.push_back(input.substr(pos, end - pos));
        pos = end;
    }
    
    if (tokens.empty())
        return (false);
    
    _command = tokens[0];
    std::cout << GREEN << "COMMAND = " << _command << RESET << std::endl;
    
    for (size_t i = 1; i < tokens.size(); ++i)
    {
        _params.push_back(tokens[i]);
    }
    
    return (true);
}

//RETURN COMMAND STR
std::string Message::getCommand() const
{
    return (_command);
}

//RETURN PARAM NBR
int Message::getParamCount() const
{
    return (_params.size());
}

void Message::printParam() const
{
    for (size_t i = 0; i < _params.size(); ++i)
    {
        std::cout << "---- param " << (i + 1) << " = " << _params[i] << std::endl;
    }
}
