#include "../../includes/channel.hpp"
#include <algorithm>


// 
Channel::Channel() : _name("") {}
Channel::Channel(const std::string& name) : _name(name), _topic(""), _modeI(false), _modeT(false), _limit(0), _key("") {}
Channel::Channel(const Channel& other) { *this = other; }
Channel& Channel::operator=(const Channel& other)
{
    if (this != &other)
    {
        _name = other._name;
        _topic = other._topic;
        _clients = other._clients;
        _operators = other._operators;
        _invited = other._invited;
        _modeI = other._modeI;
        _modeT = other._modeT;
        _key = other._key;
        _limit = other._limit;
    }
    return *this;
}
Channel::~Channel() {}



// Getters
const std::string& Channel::getName() const { return _name; }
const std::vector<Client*>& Channel::getClients() const { return _clients; }



// Adds a client to channel
void Channel::addClient(Client* client)
{
    if (!_clients.empty() && _limit > 0 && _clients.size() >= static_cast<size_t>(_limit))
        return;

    _clients.push_back(client);
}

// Removes a client from channel
void Channel::removeClient(Client* client)
{
    std::vector<Client*>::iterator i = _clients.begin();
    while (i != _clients.end())
    {
        if (*i == client)
        {
            _clients.erase(i);
            return;
        }
        ++i;
    }
	removeOperator(client);
    removeInvited(client);
}

// Checks if a client is already in channel
bool Channel::hasClient(Client* client) const
{
    for (size_t i = 0; i < _clients.size(); ++i)
        if (_clients[i] == client)
            return true;
    return false;
}

void Channel::addOperator(Client* client)
{
    if (!isOperator(client))
        _operators.push_back(client);
}

bool Channel::isOperator(Client* client) const
{
    for (size_t i = 0; i < _operators.size(); ++i)
    {
        if (_operators[i] == client)
            return true;
    }
    return false;
}

void Channel::removeOperator(Client* client)
{
    for (size_t i = 0; i < _operators.size(); ++i)
    {
        if (_operators[i] == client)
        {
            _operators.erase(_operators.begin() + i);
            break;
        }
    }
}

void Channel::addInvite(Client* client)
{
    if (!isInvited(client))
        _invited.push_back(client);
}

bool Channel::isInvited(Client* client) const
{
    for (size_t i = 0; i < _invited.size(); ++i)
    {
        if (_invited[i] == client)
            return true;
    }
    return false;
}

void Channel::removeInvited(Client* client)
{
    for (size_t i = 0; i < _invited.size(); ++i)
    {
        if (_invited[i] == client)
        {
            _invited.erase(_invited.begin() + i);
            break;
        }
    }
}

std::string Channel::getTopic() const
{
    return _topic;
}

void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}

void Channel::setModeI(bool enable)
{
    _modeI = enable;
}

void Channel::setModeT(bool enable)
{
    _modeT = enable;
}

bool Channel::isModeI() const
{
    return _modeI;
}

bool Channel::isModeT() const
{
    return _modeT;
}

std::string Channel::getKey() const
{
    return _key;
}

void Channel::setKey(const std::string& key)
{
    _key = key;
}

void Channel::setLimit(int limit)
{
    _limit = limit;
}

size_t Channel::getLimit() const
{
    return _limit;
}
