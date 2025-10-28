/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chuchard <chuchard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 23:07:37 by nihamdan          #+#    #+#             */
/*   Updated: 2025/10/28 07:01:29 by chuchard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"
#include "../includes/client.hpp"
#include "../includes/commandHandler.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <iomanip>

static void setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
        throw std::runtime_error("fcntl(F_SETFL,O_NONBLOCK) failed");
    }
}

Server::Server(int port, const std::string& password): _serverFd(-1), _port(port), _password(password)
{
    signal(SIGPIPE, SIG_IGN);
    makeServerSocket();
}

Server::~Server()
{
    if (_serverFd >= 0)
		close(_serverFd);
    for (size_t i = 0; i < _clients.size(); ++i)
	{
        close(_clients[i]->getFd());
        delete _clients[i];
    }
    /*for (size_t i = 0; i < _channels.size(); ++i) // a adapté
	{
        delete _channels[i];
    }*/
}

void Server::makeServerSocket()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0)
		throw std::runtime_error("socket() failed");

    int opt = 1;
    setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(_serverFd, 128) < 0)
        throw std::runtime_error("listen() failed");

    setNonBlocking(_serverFd);

    std::cout << "Listening on port " << _port << std::endl;
}

void Server::addNewClient()
{
    int cfd = accept(_serverFd, NULL, NULL);
    if (cfd < 0)
		return;
    setNonBlocking(cfd);

    Client* cl = new Client(cfd);
    _clients.push_back(cl);

    std::cout << "New client connected (fd=" << cfd << ")" << std::endl;
    printClients();                                                             // AJOUT PRINT LES CLIENT A CHAQUE NOUVEAU CONNECTÉ
}

void Server::dropClient(int fd)
{
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
        if ((*it)->getFd() == fd)
		{
            std::cout << "Client disconnected (fd=" << fd << ")" << std::endl;
            close(fd);
            delete *it;
            _clients.erase(it);
            return;
        }
    }
}

void Server::handleReadable(int fd)
{
    Client* cl = NULL;
    for (size_t i = 0; i < _clients.size(); ++i)
	{
        if (_clients[i]->getFd() == fd)
		{
            cl = _clients[i];
            break;
        }
    }
    if (!cl) return;

    char buf[512];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n == 0)
	{
		dropClient(fd);
		return;
	}
    if (n < 0)
	{
        if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
        dropClient(fd);
        return;
    }

    cl->appendToBuffer(buf, (size_t)n);

    // std::string line;
    // while (cl->extractLine(line))
	// {
    //     std::cout << "[fd " << fd << "] line received: " << line << std::endl;
	// 	// a voir si je l'imprime ou si tu veux directement parser avec ton split et l'imprimer ensuite (donc la stocké)    
    // }

    CommandHandler cmdHandler(this);                                            // AJOUT
    std::vector<std::string> messages = cl->getMessage();                       //
    for (size_t j = 0; j < messages.size(); ++j)                                //
        cmdHandler.handleCommand(cl, messages[j]);                              //
}

void Server::run()
{
    struct pollfd fds[1024];
    while (true)
	{
        int nfds = 0;
        fds[nfds].fd = _serverFd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        ++nfds;

        for (size_t i = 0; i < _clients.size(); ++i)
		{
            fds[nfds].fd = _clients[i]->getFd();
            fds[nfds].events = POLLIN;
            fds[nfds].revents = 0;
            ++nfds;
        }

        int ret = poll(fds, nfds, -1);
        if (ret < 0)
		{
            if (errno == EINTR)
				continue;
            throw std::runtime_error("poll() failed");
        }

        if (fds[0].revents & POLLIN)
            addNewClient();

        for (int i = 1; i < nfds; ++i)
		{
            if (fds[i].revents & POLLIN)
                handleReadable(fds[i].fd);
        }
    }
}

const std::vector<Client*>& Server::getClients() const
{
    return _clients;
}

Client* Server::getClientByNick(const std::string& nickname) const
{
    for (size_t i = 0; i < _clients.size(); ++i)
	{
        if (_clients[i]->getNickname() == nickname)
            return _clients[i];
    }
    return NULL;
}

/*Channel* Server::getChannelByName(const std::string& name) const
{
    for (size_t i = 0; i < _channels.size(); ++i)
	{
        // mettre ton getname de channel ici, je met un prototype
        // if (_channels[i]->getName() == name) return _channels[i];
    }
    return NULL;
}*/

//                                                                              // AJOUT A PARTIR D'ICI

// Finds a Channel by its name
Channel* Server::getChannelByName(const std::string& name) const
{
    for (size_t i = 0; i < _channels.size(); ++i)
        if (_channels[i]->getName() == name)
            return _channels[i];
    return NULL;
}

const std::string Server::getPassword() const { return _password; }

void Server::addChannel(Channel* ch)
{
    _channels.push_back(ch);
}

void Server::printClients() const
{
    const int fdWidth = 4;
    const int authWidth = 4;
    const int regWidth = 3;
    const int nickWidth = 12;
    const int userWidth = 10;

    std::cout << "\033[2J\033[2;1H==================== CLIENTS ====================" << std::endl;
    std::cout << "| "
              << std::setw(fdWidth) << "FD" << " | "
              << std::setw(authWidth) << "Auth" << " | "
              << std::setw(regWidth) << "Reg" << " | "
              << std::setw(nickWidth) << "Nickname" << " | "
              << std::setw(userWidth) << "Username" << " |" << std::endl;

    std::cout << "-------------------------------------------------" << std::endl;

    for (size_t i = 0; i < _clients.size(); ++i)
    {
        const Client* c = _clients[i];

        std::string nick = c->getNickname().empty() ? "None" : c->getNickname();
        std::string user = c->getUsername().empty() ? "None" : c->getUsername();

        // truncate if too long
        if (nick.length() > static_cast<size_t>(nickWidth))
            nick = nick.substr(0, nickWidth - 1);
        if (user.length() > static_cast<size_t>(userWidth))
            user = user.substr(0, userWidth - 1);

        // print each row with padding
        std::cout << "| " 
                  << std::setw(fdWidth) << c->getFd() << " | "
                  << std::setw(authWidth) << (c->isAuthenticated() ? "Yes" : "No") << " | "
                  << std::setw(regWidth) << (c->isRegistered() ? "Yes" : "No") << " | "
                  << std::setw(nickWidth) << nick << " | "
                  << std::setw(userWidth) << user << " |" << std::endl;
    }

    std::cout << "=================================================" << std::endl;
}

void Server::printChannelInfo(const Channel* target) const
{
    int nameWidth = 12;
    int topicWidth = 12;
    const int modeWidth = 1;
    const int memberWidth = 7;

    printClients();
    std::cout << std::endl;

    // === Global overview ===
    std::cout << "=================== CHANNELS OVERVIEW ===================" << std::endl;
    std::cout << "| " << std::setw(nameWidth) << "Name"
              << " | " << std::setw(topicWidth) << "Topic"
              << " | " << std::setw(modeWidth) << "i"
              << " | " << std::setw(modeWidth) << "t"
              << " | " << std::setw(modeWidth) << "k"
              << " | " << std::setw(modeWidth) << "l"
              << " | " << std::setw(memberWidth) << "Members"
              << " |" << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < _channels.size(); ++i)
    {
        Channel* ch = _channels[i];
        std::string topic = ch->getTopic().empty() ? "(none)" : ch->getTopic();
        if (topic.length() > static_cast<size_t>(topicWidth))
            topic = topic.substr(0, topicWidth - 1);

        std::cout << "| " << std::setw(nameWidth) << ch->getName()
                  << " | " << std::setw(topicWidth) << topic
                  << " | " << std::setw(modeWidth) << (ch->isModeI() ? "✓" : "")
                  << " | " << std::setw(modeWidth) << (ch->isModeT() ? "✓" : "")
                  << " | " << std::setw(modeWidth) << (!ch->getKey().empty() ? "✓" : "")
                  << " | " << std::setw(modeWidth) << (ch->getLimit() > 0 ? "✓" : "")
                  << " | " << std::setw(memberWidth) << ch->getClients().size()
                  << " |" << std::endl;
    }

    std::cout << "=========================================================" << std::endl;

    // === If no target given, stop here ===
    if (!target)
        return;

    // === Detailed view for one channel ===
    const int nickWidth = 12;
    const int userWidth = 9;
    const int memberWidth2 = 3;
    const int opWidth = 3;
    const int inviteWidth = 3;

    std::string centeredName = target->getName();
    int pad = (37 - static_cast<int>(centeredName.size())) / 2;
    if (pad < 0) pad = 0;

    std::cout << std::endl;
    std::cout << "============== MODIFIED CHANNEL ==============" << std::endl;

    const int colWidth = 12;
    std::cout << "| " << std::setw(colWidth) << "Name"
              << " | " << std::setw(colWidth) << "Topic"
              << " | " << std::setw(colWidth) << "Key"
              << " |" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;

    std::string nameVal = target->getName().empty() ? "(none)" : target->getName();
    std::string topicVal = target->getTopic().empty() ? "(none)" : target->getTopic();
    std::string keyVal = target->getKey().empty() ? "(none)" : target->getKey();

    if (nameVal.length() > static_cast<size_t>(colWidth))
        nameVal = nameVal.substr(0, colWidth - 1);
    if (topicVal.length() > static_cast<size_t>(colWidth))
        topicVal = topicVal.substr(0, colWidth - 1);
    if (keyVal.length() > static_cast<size_t>(colWidth))
        keyVal = keyVal.substr(0, colWidth - 1);

    std::cout << "| " << std::setw(colWidth) << nameVal
              << " | " << std::setw(colWidth) << topicVal
              << " | " << std::setw(colWidth) << keyVal
              << " |" << std::endl;
    std::cout << "==============================================" << std::endl;

    std::cout << "| " << std::setw(nickWidth) << "Nickname"
              << " | " << std::setw(userWidth) << "Username"
              << " | " << std::setw(memberWidth2) << "Mbr"
              << " | " << std::setw(opWidth) << "Ope"
              << " | " << std::setw(inviteWidth) << "Inv"
              << " |" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;

    const std::vector<Client*>& allClients = _clients;

    for (size_t i = 0; i < allClients.size(); ++i)
    {
        const Client* c = allClients[i];
        std::string nick = c->getNickname().empty() ? "None" : c->getNickname();
        std::string user = c->getUsername().empty() ? "None" : c->getUsername();

        if (nick.length() > static_cast<size_t>(nickWidth))
            nick = nick.substr(0, nickWidth - 1);
        if (user.length() > static_cast<size_t>(userWidth))
            user = user.substr(0, userWidth - 1);

        bool isMember = target->hasClient(const_cast<Client*>(c));
        bool isOp = target->isOperator(const_cast<Client*>(c));
        bool isInvited = target->isInvited(const_cast<Client*>(c));

        std::cout << "| " << std::setw(nickWidth) << nick
                  << " | " << std::setw(userWidth) << user
                  << " | " << std::setw(memberWidth2) << (isMember ? " ✓ " : "")
                  << " | " << std::setw(opWidth) << (isOp ? " ✓ " : "")
                  << " | " << std::setw(inviteWidth) << (isInvited ? " ✓ " : "")
                  << " |" << std::endl;
    }

    std::cout << "==============================================" << std::endl;
}
