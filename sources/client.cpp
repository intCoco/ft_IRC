/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chuchard <chuchard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 23:07:06 by nihamdan          #+#    #+#             */
/*   Updated: 2025/10/28 01:58:06 by chuchard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/client.hpp"

Client::Client(int fd): _fd(fd), _nickname(""), _username(""), _bufferIn(""), _authenticated(false), _registered(false) {}

Client::~Client() {}

int Client::getFd() const
{
	return _fd;
}

void Client::setNickname(const std::string& nick)
{
	_nickname = nick;
}

void Client::setUsername(const std::string& user)
{
	_username = user;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

void Client::setAuthenticated(bool ok)
{
	_authenticated = ok;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

void Client::setRegistered(bool ok)
{
	_registered = ok;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::appendToBuffer(const char* data, size_t len)
{
    _bufferIn.append(data, len);
    if (_bufferIn.size() > 512)
	{
        _bufferIn.erase(0, _bufferIn.size() - 512);
    }
}

bool Client::extractLine(std::string& out)
{
    //(\r\n) en fonction de l'os
    std::string::size_type pos = _bufferIn.find("\r\n");
    if (pos != std::string::npos)
	{
        out = _bufferIn.substr(0, pos);
        _bufferIn.erase(0, pos + 2);
        return true;
    }
    //(\n)
    pos = _bufferIn.find("\n");
    if (pos != std::string::npos)
	{
        out = _bufferIn.substr(0, pos);
        _bufferIn.erase(0, pos + 1);
        return true;
    }
    //(\r)
    pos = _bufferIn.find("\r");
    if (pos != std::string::npos)
	{
        out = _bufferIn.substr(0, pos);
        _bufferIn.erase(0, pos + 1);
        return true;
    }
    return false;
}


std::vector<std::string> Client::getMessage()
{
    std::vector<std::string> messages;
    const std::string delimiter = "\n";                                                                                         // [MODIFICATION] ca va pas "\r\n" car ca cherche les deux d'affilé alors que il peut y avoir simplement \n seul quand le client envoie un message. À RÉADAPTER !
    size_t pos = 0;
    
    // std::cout << BLUE << "CLIENT " << _fd << " buffer message = " << RESET << std::endl;
    
    std::string debugBuffer = _bufferIn;
    for (size_t i = 0; i < debugBuffer.length(); ++i)
    {
        if (debugBuffer[i] == '\r')
            debugBuffer.replace(i, 1, "\\r");
        else if (debugBuffer[i] == '\n')
            debugBuffer.replace(i, 1, "\\n");
    }
    // std::cout << BLUE << "CLIENT " << _fd << " current buffer = '" << debugBuffer << "'" << RESET << std::endl;
    
    while ((pos = _bufferIn.find(delimiter)) != std::string::npos)
    {
        std::string message = _bufferIn.substr(0, pos);

        if (!message.empty() && message[message.length() - 1] == '\r')
            message.erase(message.length() - 1);
            
        if (!message.empty())
            messages.push_back(message);
        
        _bufferIn.erase(0, pos + 1);
    }

    // std::cout << BLUE << "CLIENT " << _fd << " OK " << messages.size() 
            //   << " messages. NEXT = '" << _bufferIn << "'" << RESET << std::endl;
    
    return (messages);
}