#include "../includes/commandHandler.hpp"
#include "../includes/errors.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cctype>
#include <cstring>
#include <stdlib.h>
#include <algorithm>

CommandHandler::CommandHandler(Server* server) : _server(server) {}
CommandHandler::CommandHandler(const CommandHandler& other) { (void)other; }
CommandHandler& CommandHandler::operator=(const CommandHandler& other) { (void)other; return *this; }
CommandHandler::~CommandHandler() {}





// // ----- Split function -----
// // Split a line into words separated by spaces
// std::vector<std::string> CommandHandler::split(const std::string& line) const
// {
//     std::vector<std::string> tokens;
//     std::istringstream iss(line);          // Use string stream to read words
//     std::string word;
//     while (iss >> word)                    // Read word by word
//         tokens.push_back(word);
//     return tokens;
// }

std::vector<std::string> CommandHandler::split(const std::string& line) const
{
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string word;

    while (iss >> word)
    {
        if (word[0] == ':') // Takes the rest of the line as a single token
        {
            std::string rest;
            std::getline(iss, rest);
            if (!rest.empty())
                word += rest; // takes everything after the ':'

            if (word.size() > 1 && word[0] == ':') // Remove the ':' before pushing
                word.erase(0, 1);

            tokens.push_back(word); // pushes everything as last parameter
            break;
        }
        tokens.push_back(word);
    }

    return tokens;
}

std::string toLower(const std::string& str)
{
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}






// ----- Handle command -----
// Parse the command and call the corresponding function
void CommandHandler::handleCommand(Client* client, const std::string& line)
{
    std::vector<std::string> tokens = split(line);
    if (tokens.empty())
        return;

    std::string cmd = tokens[0];
    for (size_t i = 0; i < cmd.size(); ++i) // put everything in upper case to allow different letter cases
        cmd[i] = std::toupper(cmd[i]);

    if (tokens.size() >= 2 && cmd == "CAP" && tokens[1] == "LS")
    {
        std::string ok = ":ircserv CAP * LS :\r\n";
        send(client->getFd(), ok.c_str(), ok.size(), 0);
        return ;
    }
    if (line == "JOIN :" || cmd == "CAP")
        return ;
    if (!client->isAuthenticated() && cmd != "PASS")
    {
        send(client->getFd(), "Error: you must enter the password first\r\n", 42, 0);
        return;
    }
    if (!client->isRegistered() && cmd != "NICK" && cmd != "USER" && cmd != "PASS") // Forces registration before continuing
    {
        send(client->getFd(), ERR_NOTREGISTERED, strlen(ERR_NOTREGISTERED), 0);
        return;
    }

    if (cmd == "PASS")
        cmdPass(client, tokens);
    else if (cmd == "NICK")
        cmdNick(client, tokens);
    else if (cmd == "USER")
        cmdUser(client, tokens);
    else if (cmd == "JOIN")
        cmdJoin(client, tokens);
    else if (cmd == "PRIVMSG")
        cmdPrivmsg(client, tokens);
    else if (cmd == "KICK")
        cmdKick(client, tokens);
    else if (cmd == "INVITE")
        cmdInvite(client, tokens);
    else if (cmd == "TOPIC")
        cmdTopic(client, tokens);
    else if (cmd == "MODE")
    {
        if (tokens[1][0] != '#' && tokens[1] == client->getNickname() && tokens[2] == "+i")
        {
            std::string reply = ":ircserv 221 " + client->getNickname() + " +i\r\n";
            send(client->getFd(), reply.c_str(), reply.size(), 0);
        }
        else
            cmdMode(client, tokens);
    }
    else if (cmd == "PING")
    {
        std::string response = "PONG";
        if (tokens.size() >= 2)
            response += " :" + tokens[1];
        response += "\r\n";

        send(client->getFd(), response.c_str(), response.size(), 0);
    }
    else if (cmd == "WHOIS")
        return ;
    else
        send(client->getFd(), ERR_UNKNOWNCOMMAND, strlen(ERR_UNKNOWNCOMMAND), 0);
}






// ----- Command implementations -----
// Each function handles a basic IRC command



// Handles PASS command
void CommandHandler::cmdPass(Client* client, const std::vector<std::string>& args)
{
    if (!client->isAuthenticated())
    {
        for (size_t j = 1; j < args.size(); ++j) {

            std::string trimmed = args[j];

            if (!trimmed.empty() && trimmed[trimmed.size() - 1] == '\n')
                trimmed.erase(trimmed.size() - 1, 1);
            if (!trimmed.empty() && trimmed[trimmed.size() - 1] == '\r')
                trimmed.erase(trimmed.size() - 1, 1);

            if (trimmed == _server->getPassword())
            {
                client->setAuthenticated(true);
                std::string ok = ":ircserv NOTICE * :Password accepted\r\n";
                send(client->getFd(), ok.c_str(), ok.size(), 0);
                _server->printClients();
                return;
            }
            else
            {
                std::string fail = ":ircserv 464 " + client->getNickname() + " :Password incorrect\r\n";
                send(client->getFd(), fail.c_str(), fail.size(), 0);
                return;
            }
        }
    }
}

// Handles NICK command
void CommandHandler::cmdNick(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        send(client->getFd(), ERR_NONICKNAMEGIVEN, strlen(ERR_NONICKNAMEGIVEN), 0);
        return;
    }

    std::string nickname = toLower(args[1]);
    if (isNicknameTaken(nickname))
    {
        if (client->isRegistered())
        {
            std::string msg = "433 " + nickname + " :Nickname is already in use\r\n";
            send(client->getFd(), msg.c_str(), msg.size(), 0);
        }
        else
        {
            std::string newNick = nickname;
            int i = 1;
            while (isNicknameTaken(newNick))
            {
                newNick = nickname + std::string("_") + static_cast<char>('0' + i);
                i++;
            }
            client->setNickname(newNick);
            std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost NICK :" + newNick + "\r\n";
            send(client->getFd(), msg.c_str(), msg.size(), 0);
        }
        _server->printClients();
        return;
    }
    client->setNickname(nickname);
    std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost NICK :" + nickname + "\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0);
    if (!client->getUsername().empty() && !client->isRegistered()) // complete registration
        sendWelcome(client);
    _server->printClients();
}

// Handles USER command
void CommandHandler::cmdUser(Client* client, const std::vector<std::string>& args)
{
    if (client->isRegistered())
    {
        send(client->getFd(), ERR_ALREADYREGISTRED, strlen(ERR_ALREADYREGISTRED), 0);
        return;
    }

    if (args.size() < 5) // USER <username> <hostname> <servername> :<realname>
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    std::string username = args[1];
    std::string realname = args[4];

    client->setUsername(username);
    client->setRealname(realname);

    if (!client->getNickname().empty() && !client->isRegistered())
        sendWelcome(client);

    _server->printClients();
}




// Handles JOIN command
void CommandHandler::cmdJoin(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    std::string channelName = toLower(args[1]);
    std::string keyParam = "";
    if (args.size() >= 3)
        keyParam = args[2];

    Channel* ch = _server->getChannelByName(channelName);
    if (!ch)
    {
        ch = new Channel(channelName);
        _server->addChannel(ch);
        ch->addClient(client);
        ch->addOperator(client);

        // INTRO WHEN CREATING CHANNEL
        std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() +
                              "@localhost JOIN :" + channelName + "\r\n";
        send(client->getFd(), joinMsg.c_str(), joinMsg.size(), 0);

        std::string topicMsg = ":ircserv 332 " + client->getNickname() + " " +
                               channelName + " :" + ch->getTopic() + "\r\n";
        send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);

        std::string names = ":ircserv 353 " + client->getNickname() + " = " +
                            channelName + " :@" + client->getNickname() + "\r\n";
        send(client->getFd(), names.c_str(), names.size(), 0);

        std::string endNames = ":ircserv 366 " + client->getNickname() + " " +
                               channelName + " :End of /NAMES list\r\n";
        send(client->getFd(), endNames.c_str(), endNames.size(), 0);

        _server->printChannelInfo(ch);
        return;
    }

    // check invite
    if (ch->isModeI() && !ch->isInvited(client) && !ch->isOperator(client))
    {
        std::string err = ":ircserv 473 " + client->getNickname() + " " +
                          channelName + " :Cannot join channel (+i)\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    // check pw
    if (!ch->getKey().empty())
    {
        if (keyParam.empty() || keyParam != ch->getKey())
        {
            std::string err = ":ircserv 475 " + client->getNickname() + " " +
                              channelName + " :Cannot join channel (+k)\r\n";
            send(client->getFd(), err.c_str(), err.size(), 0);
            return;
        }
    }

    // check limit
    if (ch->getLimit() > 0 && ch->getClients().size() >= ch->getLimit())
    {
        std::string err = ":ircserv 471 " + client->getNickname() + " " +
                          channelName + " :Cannot join channel (+l)\r\n";
        send(client->getFd(), err.c_str(), err.size(), 0);
        return;
    }

    // if already member
    if (ch->hasClient(client))
        return;

    ch->addClient(client);
    ch->removeInvited(client);

    // announcement to everyone
    std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() +
                          "@localhost JOIN :" + channelName + "\r\n";
    const std::vector<Client*>& members = ch->getClients();
    for (size_t i = 0; i < members.size(); ++i)
        send(members[i]->getFd(), joinMsg.c_str(), joinMsg.size(), 0);


    // INTRO WHEN ARRIVING IN CHANNEL
    std::string topicMsg = ":ircserv 332 " + client->getNickname() + " " +
                           channelName + " :" + ch->getTopic() + "\r\n";
    send(client->getFd(), topicMsg.c_str(), topicMsg.size(), 0);

    // names list
    std::string names = ":ircserv 353 " + client->getNickname() + " = " + channelName + " :";
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (ch->isOperator(members[i]))
            names += "@" + members[i]->getNickname() + " ";
        else
            names += members[i]->getNickname() + " ";
    }
    names += "\r\n";
    send(client->getFd(), names.c_str(), names.size(), 0);

    // end of names list
    std::string endNames = ":ircserv 366 " + client->getNickname() + " " +
                           channelName + " :End of /NAMES list\r\n";
    send(client->getFd(), endNames.c_str(), endNames.size(), 0);

    _server->printChannelInfo(ch);
}



// Handles PRIVMSG command
void CommandHandler::cmdPrivmsg(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    std::string targetName = toLower(args[1]);
    std::string message = args[2];

    bool sent = false;
    const std::vector<Client*>& clients = _server->getClients();
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getNickname() == targetName) // search target
        {
            std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PRIVMSG " + targetName + " :" + message + "\r\n";
            send(clients[i]->getFd(), msg.c_str(), msg.size(), 0);
            sent = true;
            break;
        }
    }

    if (!sent)
    {
        Channel* ch = _server->getChannelByName(targetName);
        if (ch)
        {
            // [FIX] Check if sender is in channel
            if (!ch->hasClient(client))
            {
                std::string err = ":ircserv 404 " + client->getNickname() + " " + targetName + " :Cannot send to channel\r\n";
                send(client->getFd(), err.c_str(), err.size(), 0);
                return;
            }

            const std::vector<Client*>& chClients = ch->getClients();
            for (size_t i = 0; i < chClients.size(); ++i)
            {
                if (chClients[i] != client) // Send to all channel clients except sender
                {
                    std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost PRIVMSG " + targetName + " :" + message + "\r\n";
                    send(chClients[i]->getFd(), msg.c_str(), msg.size(), 0);
                }
            }
            sent = true;
        }
    }

    if (!sent)
    {
        send(client->getFd(), ERR_NOSUCHNICK, strlen(ERR_NOSUCHNICK), 0);
    }
}


// Remove a client from a channel
void CommandHandler::cmdKick(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 3) // Need channel + target
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    Channel* ch = _server->getChannelByName(toLower(args[1]));
    if (!ch)
    {
        send(client->getFd(), ERR_NOSUCHCHANNEL, strlen(ERR_NOSUCHCHANNEL), 0);
        return;
    }

    if (!ch->isOperator(client)) // Only operators can kick
    {
        send(client->getFd(), ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0);
        return;
    }

    Client* target = _server->getClientByNick(toLower(args[2]));
    if (!target || !ch->hasClient(target)) // Check target exists in channel
    {
        send(client->getFd(), ERR_USERNOTINCHANNEL, strlen(ERR_USERNOTINCHANNEL), 0);
        return;
    }

    ch->removeClient(target); // Remove target from channel

    std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " + ch->getName() + " " + target->getNickname() + " :Kicked\r\n";

    const std::vector<Client*>& members = ch->getClients();
    for (size_t i = 0; i < members.size(); ++i)
        send(members[i]->getFd(), kickMsg.c_str(), kickMsg.size(), 0); //send to everyone
    send(target->getFd(), kickMsg.c_str(), kickMsg.size(), 0);

    _server->printChannelInfo(ch);
}


// Invite a client to a channel
void CommandHandler::cmdInvite(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    Channel* ch = _server->getChannelByName(toLower(args[2]));
    if (!ch)
    {
        send(client->getFd(), ERR_NOSUCHCHANNEL, strlen(ERR_NOSUCHCHANNEL), 0);
        return;
    }

    if (!ch->isOperator(client))
    {
        send(client->getFd(), ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0);
        return;
    }

    Client* target = _server->getClientByNick(toLower(args[1]));
    if (!target)
    {
        send(client->getFd(), ERR_NOSUCHNICK, strlen(ERR_NOSUCHNICK), 0);
        return;
    }

    ch->addInvite(target); // Add to invitation list
    std::string invMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost INVITE " + target->getNickname() + " :" + ch->getName() + "\r\n";
    send(target->getFd(), invMsg.c_str(), invMsg.size(), 0);
    _server->printChannelInfo(ch);
}


// View or change channel topic
void CommandHandler::cmdTopic(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    Channel* ch = _server->getChannelByName(toLower(args[1]));
    if (!ch)
    {
        send(client->getFd(), ERR_NOSUCHCHANNEL, strlen(ERR_NOSUCHCHANNEL), 0);
        return;
    }

    // Just display current topic
    if (args.size() == 2)
    {
        std::string msg = "Topic for " + ch->getName() + ": " + ch->getTopic() + "\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    if (ch->isModeT() && !ch->isOperator(client))
    {
        send(client->getFd(), ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0);
        return;
    }

    ch->setTopic(args[2]);

    std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
                    "@localhost TOPIC " + ch->getName() + " :" + args[2] + "\r\n";

    const std::vector<Client*>& members = ch->getClients();
    for (size_t i = 0; i < members.size(); ++i)
        send(members[i]->getFd(), msg.c_str(), msg.size(), 0);

    _server->printChannelInfo(ch);

}


// Handles MODE command  
// MODE <channel> <modes> [params...]
void CommandHandler::cmdMode(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    std::string channelName = toLower(args[1]);
    Channel* ch = _server->getChannelByName(channelName);
    if (!ch)
    {
        send(client->getFd(), ERR_NOSUCHCHANNEL, strlen(ERR_NOSUCHCHANNEL), 0);
        return;
    }

    if (!ch->isOperator(client)) // Only operators can change modes
    {
        send(client->getFd(), ERR_CHANOPRIVSNEEDED, strlen(ERR_CHANOPRIVSNEEDED), 0);
        return;
    }

    std::string modes = args[2];
    size_t argIndex = 3;

    bool enable = true;
    for (size_t i = 0; i < modes.size(); ++i)
    {
        char c = modes[i];

        if (c == '+') { enable = true; continue; }
        if (c == '-') { enable = false; continue; }

        if (c == 'i') // Invite-only
        {
            ch->setModeI(enable);
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (enable ? "+" : "-") + c + "\r\n";
            const std::vector<Client*>& members = ch->getClients();
            for (size_t i = 0; i < members.size(); ++i)
                send(members[i]->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
        }
        else if (c == 't') // Topic only by ops (si j'ai bien compris?)
        {
            ch->setModeT(enable);
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (enable ? "+" : "-") + c + "\r\n";
            const std::vector<Client*>& members = ch->getClients();
            for (size_t i = 0; i < members.size(); ++i)
                send(members[i]->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
        }
        else if (c == 'k') // Channel key (password)
        {
            if (enable)
            {
                if (argIndex >= args.size())
                {
                    send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
                    _server->printChannelInfo(ch);
                    return;
                }
                ch->setKey(args[argIndex++]);
            }
            else
                ch->setKey("");

            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (enable ? "+" : "-") + c + "\r\n";
            const std::vector<Client*>& members = ch->getClients();
            for (size_t i = 0; i < members.size(); ++i)
                send(members[i]->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
        }
        else if (c == 'o') // Set/remove operator
        {
            if (argIndex >= args.size())
            {
                send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
                _server->printChannelInfo(ch);
                return;
            }
            Client* target = _server->getClientByNick(toLower(args[argIndex++]));
            if (!target || !ch->hasClient(target))
            {
                send(client->getFd(), ERR_USERNOTINCHANNEL, strlen(ERR_USERNOTINCHANNEL), 0);
                _server->printChannelInfo(ch);
                return;
            }
            if (enable)
            {
                ch->addOperator(target);
                std::string msg = target->getNickname() + " is now an operator in " + channelName + "\r\n";
                send(client->getFd(), msg.c_str(), msg.size(), 0);
            }
            else
            {
                ch->removeOperator(target);
                std::string msg = target->getNickname() + " is no longer an operator in " + channelName + "\r\n";
                send(client->getFd(), msg.c_str(), msg.size(), 0);
            }
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (enable ? "+" : "-") + c + "\r\n";
            const std::vector<Client*>& members = ch->getClients();
            for (size_t i = 0; i < members.size(); ++i)
                send(members[i]->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
        }
        else if (c == 'l') // Set/remove user limit
        {
            if (enable)
            {
                if (argIndex >= args.size())
                {
                    send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
                    _server->printChannelInfo(ch);
                    return;
                }
                int limit = atoi(args[argIndex++].c_str());
                ch->setLimit(limit);
                std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (enable ? "+" : "-") + c + "\r\n";
                const std::vector<Client*>& members = ch->getClients();
                for (size_t i = 0; i < members.size(); ++i)
                    send(members[i]->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
            }
            else
                ch->setLimit(0);
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost MODE " + channelName + " " + (enable ? "+" : "-") + c + "\r\n";
            const std::vector<Client*>& members = ch->getClients();
            for (size_t i = 0; i < members.size(); ++i)
                send(members[i]->getFd(), modeMsg.c_str(), modeMsg.size(), 0);
        }
        else
        {
            send(client->getFd(), ERR_UNKNOWNCOMMAND, strlen(ERR_UNKNOWNCOMMAND), 0); // unsupported mode
        }
    }
    _server->printChannelInfo(ch);
}







// ----- Utils -----
void CommandHandler::sendWelcome(Client* client)
{
    client->setRegistered(true);
    std::string msg = ":ircserv 001 " + client->getNickname() + " :Welcome to the Internet Relay Network\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0);
}

bool CommandHandler::isNicknameTaken(const std::string& nickname) const
{
    const std::vector<Client*>& clients = _server->getClients();
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getNickname() == toLower(nickname))
            return true;
    }
    return false;
}
