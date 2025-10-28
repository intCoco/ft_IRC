#include "../includes/commandHandler.hpp"
#include "../includes/errors.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <cctype>
#include <cstring>
#include <stdlib.h>

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







// ----- Handle command -----
// Parse the command and call the corresponding function
void CommandHandler::handleCommand(Client* client, const std::string& line)
{
    std::vector<std::string> tokens = split(line);
    if (tokens.empty())
        return;

    std::string cmd = tokens[0];
    // for (size_t i = 0; i < cmd.size(); ++i) // put everything in upper case to allow different letter cases
    //     cmd[i] = std::toupper(cmd[i]);

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
        cmdMode(client, tokens);
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

            if (trimmed == this->_server->_password)
            {
                client->setAuthenticated(true);
                std::string ok = "Password correct. Welcome!\r\n";
                send(client->getFd(), ok.c_str(), ok.size(), 0);
                _server->printClients();
                return;
            }
            else
            {
                std::string fail = "Password incorrect. Try again.\r\n";
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

    std::string nickname = args[1];
    if (isNicknameTaken(nickname))
    {
        std::string msg = "433 " + nickname + " :Nickname is already in use\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        return;
    }

    client->setNickname(nickname);
    if (!client->getUsername().empty() && !client->isRegistered()) // complete registration
        sendWelcome(client);
    _server->printClients();
}


// Handles USER command
void CommandHandler::cmdUser(Client* client, const std::vector<std::string>& args)
{
    if (client->isRegistered()) // USER cannot be changed after registration
    {
        send(client->getFd(), ERR_ALREADYREGISTRED, strlen(ERR_ALREADYREGISTRED), 0);
        return;
    }

    if (args.size() < 2)
    {
        send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
        return;
    }

    client->setUsername(args[1]);
    if (!client->getNickname().empty() && !client->isRegistered()) // complete registration
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

    std::string channelName = args[1];
    std::string keyParam = ""; // optional key param (JOIN #channel key)
    if (args.size() >= 3)
        keyParam = args[2];

    Channel* ch = _server->getChannelByName(channelName);
    if (!ch)
    {
        ch = new Channel(channelName);
        _server->addChannel(ch);
        ch->addClient(client);
        ch->addOperator(client);
        std::string msg = "Joined channel " + channelName + "\r\n";
        send(client->getFd(), msg.c_str(), msg.size(), 0);
        _server->printChannelInfo(ch);
        return;
    }

    // If channel is invite-only and client is not invited and not an operator -> reject
    if (ch->isModeI() && !ch->isInvited(client) && !ch->isOperator(client))
    {
        send(client->getFd(), ERR_INVITEONLYCHAN, strlen(ERR_INVITEONLYCHAN), 0);
        return;
    }

    // If channel has a password, require correct key param
    if (!ch->getKey().empty())
    {
        if (keyParam.empty() || keyParam != ch->getKey())
        {
            send(client->getFd(), ERR_BADCHANNELKEY, strlen(ERR_BADCHANNELKEY), 0);
            return;
        }
    }

    // If channel has a user limit and it's full -> reject
    if (ch->getLimit() > 0 && ch->getClients().size() >= ch->getLimit())
    {
        send(client->getFd(), ERR_CHANNELISFULL, strlen(ERR_CHANNELISFULL), 0);
        return;
    }

    // All checks passed -> join
    ch->addClient(client);
    ch->removeInvited(client);
    std::string msg = "Joined channel " + channelName + "\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0);
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

    std::string targetName = args[1];
    std::string message = args[2];

    bool sent = false;
    const std::vector<Client*>& clients = _server->getClients();
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getNickname() == targetName) // search target
        {
            std::string msg = "Private message from " + client->getNickname() + ": " + message + "\r\n";
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
            const std::vector<Client*>& chClients = ch->getClients();
            for (size_t i = 0; i < chClients.size(); ++i)
            {
                if (chClients[i] != client) // Send to all channel clients except sender
                {
                    std::string msg = "Message from " + client->getNickname() + " to channel " + targetName + ": " + message + "\r\n";
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

    Channel* ch = _server->getChannelByName(args[1]);
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

    Client* target = _server->getClientByNick(args[2]);
    if (!target || !ch->hasClient(target)) // Check target exists in channel
    {
        send(client->getFd(), ERR_USERNOTINCHANNEL, strlen(ERR_USERNOTINCHANNEL), 0);
        return;
    }

    ch->removeClient(target); // Remove target from channel

    std::string msg = "Kicked " + target->getNickname() + " from " + ch->getName() + "\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0); // Confirm to operator
    msg = "You've been kicked by " + client->getNickname() + " from " + ch->getName() + "\r\n";
    send(target->getFd(), msg.c_str(), msg.size(), 0); // Inform kicked client
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

    Channel* ch = _server->getChannelByName(args[2]);
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

    Client* target = _server->getClientByNick(args[1]);
    if (!target)
    {
        send(client->getFd(), ERR_NOSUCHNICK, strlen(ERR_NOSUCHNICK), 0);
        return;
    }

    ch->addInvite(target); // Add to invitation list
    std::string msg = "Invited " + target->getNickname() + " to " + ch->getName() + "\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0);
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

    Channel* ch = _server->getChannelByName(args[1]);
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
    std::string msg = "Topic changed to: " + args[2] + "\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0);
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

    std::string channelName = args[1];
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
            std::string msg = enable ?
                "Channel " + channelName + " set to invite-only\r\n" :
                "Channel " + channelName + " is no longer invite-only\r\n";
            send(client->getFd(), msg.c_str(), msg.size(), 0);
        }
        else if (c == 't') // Topic only by ops (si j'ai bien compris?)
        {
            ch->setModeT(enable);
            std::string msg = enable ?
                "Only channel operators may set the topic for " + channelName + "\r\n" :
                "All users may now set the topic for " + channelName + "\r\n";
            send(client->getFd(), msg.c_str(), msg.size(), 0);
        }
        else if (c == 'k') // Channel key (password)
        {
            if (enable)
            {
                if (argIndex >= args.size())
                {
                    send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
                    return;
                }
                ch->setKey(args[argIndex++]);
                std::string msg = "Password set for channel " + channelName + "\r\n";
                send(client->getFd(), msg.c_str(), msg.size(), 0);
            }
            else
            {
                ch->setKey("");
                std::string msg = "Password removed for channel " + channelName + "\r\n";
                send(client->getFd(), msg.c_str(), msg.size(), 0);
            }
        }
        else if (c == 'o') // Set/remove operator
        {
            if (argIndex >= args.size())
            {
                send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
                return;
            }
            Client* target = _server->getClientByNick(args[argIndex++]);
            if (!target || !ch->hasClient(target))
            {
                send(client->getFd(), ERR_USERNOTINCHANNEL, strlen(ERR_USERNOTINCHANNEL), 0);
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
        }
        else if (c == 'l') // Set/remove user limit
        {
            if (enable)
            {
                if (argIndex >= args.size())
                {
                    send(client->getFd(), ERR_NEEDMOREPARAMS, strlen(ERR_NEEDMOREPARAMS), 0);
                    return;
                }
                int limit = atoi(args[argIndex++].c_str());
                ch->setLimit(limit);
                std::stringstream ss;
    		    ss << "Channel " << channelName << " limit set to " << limit << "\r\n";
    		    std::string msg = ss.str();

    		    send(client->getFd(), msg.c_str(), msg.size(), 0);
            }
            else
            {
                ch->setLimit(0);
                std::string msg = "Channel " + channelName + " limit removed\r\n";
                send(client->getFd(), msg.c_str(), msg.size(), 0);
            }
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
    std::string msg = "001 " + client->getNickname() + " :Welcome to the IRC server!\r\n";
    send(client->getFd(), msg.c_str(), msg.size(), 0);
}


bool CommandHandler::isNicknameTaken(const std::string& nickname) const
{
    const std::vector<Client*>& clients = _server->getClients();
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getNickname() == nickname)
            return true;
    }
    return false;
}
