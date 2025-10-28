/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chuchard <chuchard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 23:07:06 by nihamdan          #+#    #+#             */
/*   Updated: 2025/10/28 18:13:59 by chuchard         ###   ########.fr       */
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

void Client::setRealname(const std::string& name) { _realname = name; }         // AJOUT

const std::string& Client::getNickname() const
{
	return _nickname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

const std::string& Client::getRealname() const { return _realname; }            // AJOUT

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
