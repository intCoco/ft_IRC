#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <string>
#include <vector>
#include "client.hpp"
#include "server.hpp"
#include "channel.hpp"

class CommandHandler
{
public:
    CommandHandler(Server* server);
    CommandHandler(const CommandHandler& other);
    CommandHandler& operator=(const CommandHandler& other);
    ~CommandHandler();

    void handleCommand(Client* client, const std::string& line);

private:
    Server* _server; // to access clients and channels

    std::vector<std::string> split(const std::string& line) const;

    // Basic commands
    void cmdNick(Client* client, const std::vector<std::string>& args);
    void cmdUser(Client* client, const std::vector<std::string>& args);
    void cmdJoin(Client* client, const std::vector<std::string>& args);
    void cmdPrivmsg(Client* client, const std::vector<std::string>& args);

    // Operator commands
    void cmdKick(Client* client, const std::vector<std::string>& args);
    void cmdInvite(Client* client, const std::vector<std::string>& args);
    void cmdTopic(Client* client, const std::vector<std::string>& args);
    void cmdMode(Client* client, const std::vector<std::string>& args);

    // Utils
    void sendWelcome(Client* client);
    bool isNicknameTaken(const std::string& nickname) const;
};

#endif
