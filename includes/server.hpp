#ifndef SERVER_HPP
# define SERVER_HPP

#include "check.hpp"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> 
#include "client.hpp"
#include "channel.hpp"

class Server
{
    private:
        int _port;
        std::string _password;
        int _socket;
        std::vector<Client*> _clients;
        std::vector<Channel*> _channels;                                        // [AJOUTÉ] Liste de tous les channels

    public:
        Server();
        Server(int port, const std::string &password);
        ~Server();
        Server(const Server &copy);
        Server& operator=(const Server &copy);

        void makeSocket();
        void exec();
        const std::vector<Client*>& getClients() const;                         // [MODIFICATION] Décommenté et modifié pour CommandHandler

        void parseMessage(const std::string &input);

        void makeServer();
        void closePort();

        Client* getClientByNick(const std::string& nickname) const;             // [AJOUTÉ] Trouve un client
        Channel* getChannelByName(const std::string& name) const;               // [AJOUTÉ] Trouve un channel
        void addChannel(Channel* ch);                                           // [AJOUTÉ] Crée un nouveau channel

        void printClients() const;                                              // [AJOUTÉ] fonction debug pour print les clients actuels
};

#endif
