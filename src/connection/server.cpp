#include "../../includes/check.hpp"
#include "../../includes/server.hpp"
#include "../../includes/message.hpp"  
#include "../../includes/client.hpp"
#include "../../includes/auth.hpp"
#include "../../includes/commandHandler.hpp"
#include <iomanip>

//TO DO LIST
//1 - lancer le serveur (OK)
//2 - sockets (OK)
//3 - connexions (OK)
//4 LISTEN (OK)
//BIND (OK)
//SELECT (OK)

Server::Server() : _port(0), _password(""), _socket(-1)
{
    //std::cout << YELLOW << "SERVER CONSTRUCTOR" << RESET << std::endl;
}

Server::~Server()
{
    //std::cout << YELLOW << "SERVER DESTRUCTOR" << RESET << std::endl;
    if (_socket != -1)
    {
        close(_socket);
        _socket = -1;
    }

    for (size_t i = 0; i < _channels.size(); ++i)                                                                               // [AJOUTÉ] free les channels
        delete _channels[i];
    _channels.clear();
}

Server::Server(int port, const std::string& password) : _port(port), _password(password), _socket(-1)
{
    std::cout << BLUE << "---- port = " << _port << RESET << std::endl;
}

Server::Server(const Server &copy)
{
    _port = copy._port;
    _password = copy._password;
    _socket = -1;
}

Server& Server::operator=(const Server &copy)
{
    if (this != &copy)
    {
        _port = copy._port;
        _password = copy._password;
        _socket = -1;
    }
    return (*this);
}


const std::vector<Client*>& Server::getClients() const
{
    return _clients;
}


void Server::exec()
{
    //std::cout << GREEN << "--- SERVER STARTING ---" << RESET << std::endl;
    makeServer();

    //std::cout << "Password = " << _password << std::endl;
    std::cout << GREEN << "--- SERVER OK ---" << RESET << std::endl;
}

void Server::makeSocket()
{
    std::cout << GREEN << "- SOCKET CREATION START" << RESET << std::endl;
    //std::cout << BLUE << "-- TRY MAKING SOCKET" << RESET << std::endl;
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
    {
        std::cerr << RED << "--- ERROR: socket creation failed" << RESET << std::endl;
        return ;
    }
    std::cout << GREEN << "--- Socket OK fd = " << _socket << RESET << std::endl;

    closePort();
    //BIND SECTION
    //std::cout << BLUE << "- BIND START" << RESET << std::endl;
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_port);

    //std::cout << BLUE << "-- TRY MAKING BIND" << RESET << std::endl;
    if (bind(_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << RED << "--- ERROR: bind failed port = " << _port << RESET << std::endl;
        close(_socket);
        _socket = -1;
        return ;
    }
    //std::cout << GREEN << "--- BIND OK port = " << _port << RESET << std::endl;
    
    //LISTEN SECTION
    //std::cout << BLUE << "- LISTEN START" << RESET << std::endl;
    //std::cout << BLUE << "-- TRY MAKING LISTEN" << RESET << std::endl;
    if (listen(_socket, 5) == -1)
    {
        std::cerr << RED << "--- ERROR: listen failed" << RESET << std::endl;
        return ;
    }
    //std::cout << GREEN << "--- LISTEN OK port = " << _port << RESET << std::endl;
    //std::cout << GREEN << "---- SOCKET SETUP COMPLETE" << RESET << std::endl;
}

void Server::parseMessage(const std::string& input)
{
    //std::cout << YELLOW << "- PARSING START" << RESET << std::endl;
    
    Message ircMessage;
    if (ircMessage.deleteSpace(input))
    {
        std::string command = ircMessage.getCommand();
        std::cout << GREEN << "-- parsed command = " << command << RESET << std::endl;
    
        ircMessage.printParam();
    }
    else
        std::cout << RED << "-- ERROR: parsing message" << input << RESET << std::endl;
}

void Server::makeServer()
{
    //std::cout << GREEN << "- SERVER INIT" << RESET << std::endl;

    makeSocket();

    // std::cout << "MDP EST : " << _password << std::endl;
    Auth auth(_password);                                                                                                                 // [AJOUT] Crée un objet Auth pour gérer la demande et la vérification du mot de passe
    CommandHandler cmdHandler(this);                                                                                                            // [AJOUT] Crée un objet cmdHandler pour gérer les commandes IRC apres l'Auth
    //std::cout << BLUE << "-- STARTING SERVER LOOP" << RESET << std::endl;

    while (true)
    {
        // SELECT PREPARATION SECTION
        //std::cout << YELLOW << "- SELECT PREP START" << RESET << std::endl;
        fd_set readfds;
        FD_ZERO(&readfds);
        //std::cout << "-- FD_ZERO done" << std::endl;

        FD_SET(_socket, &readfds);
        //std::cout << "--- server socket " << _socket << " added to readfds" << std::endl;
        
        int maxfd = _socket;
        //std::cout << "---- initial maxfd = " << maxfd << std::endl;

        //std::cout << "- CLIENT FD ADDING" << std::endl;
        //std::cout << "-- clients count = " << _clients.size() << std::endl;
        for (size_t i = 0; i < _clients.size(); ++i)
        {
            //std::cout << "--- adding client " << i << " fd = " << _clients[i]->getFd() << " to readfds" << std::endl;
            FD_SET(_clients[i]->getFd(), &readfds);
            if (_clients[i]->getFd() > maxfd)
            {
                maxfd = _clients[i]->getFd();
                //std::cout << "---- new maxfd = " << maxfd << std::endl;
            }
        }

        // SELECT CALL SECTION
        //std::cout << YELLOW << "- SELECT CALL" << RESET << std::endl;
        //std::cout << "-- calling select with maxfd = " << maxfd << std::endl;
        int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        //std::cout << GREEN << "--- select returned = " << activity << RESET << std::endl;
        if (activity <= 0)                                                                                                               // [MODIFICATION] j'ai inversé le sens de la condition pour plus de lisibilité
        {
            std::cout << RED << "- SELECT ERROR" << RESET << std::endl;      

            std::cout << GREEN << "- LOOP ITERATION OK" << RESET << std::endl;
            std::cout << "********************" << std::endl;
            continue;
        }

        // NEW CONNECTION SECTION
        //std::cout << BLUE << "- NEW CONNECTION CHECK" << RESET << std::endl;
        if (FD_ISSET(_socket, &readfds))
        {
            //std::cout << "-- new connection detected" << std::endl;
            int newClient = accept(_socket, NULL, NULL);
            if (newClient != -1)
            {
                Client* clientObj = new Client(newClient);
                _clients.push_back(clientObj);

                auth.requestPassword(clientObj);                                                                                        // [AJOUT] Envoie le prompt au client
                //std::cout << GREEN << "--- new client created fd = " << newClient << RESET << std::endl;
                //std::cout << "---- total clients = " << _clients.size() << std::endl;
            }
            else
                std::cout << RED << "--- accept failed" << RESET << std::endl;
        }
        else
            std::cout << "-- no new connection" << std::endl;

        // CLIENT MESSAGE SECTION
        //std::cout << BLUE << "- CLIENT MESSAGE CHECK" << RESET << std::endl;
        for (size_t i = 0; i < _clients.size(); ++i)
        {
            //std::cout << "-- checking client " << i << " fd = " << _clients[i]->getFd() << std::endl;
            if (!FD_ISSET(_clients[i]->getFd(), &readfds))                                                                              // [MODIFICATION] Idem
            {
                std::cout << "--- client " << _clients[i]->getFd() << " has no data" << std::endl;
                continue;
            }
            //std::cout << PINK << "--- client " << _clients[i]->getFd() << " has data" << RESET << std::endl;

            // RECV SECTION
            //std::cout << "- RECV DATA" << std::endl;
            char buffer[512];
            int bytesReceived = recv(_clients[i]->getFd(), buffer, sizeof(buffer) - 1, 0);
            //std::cout << "-- recv returned = " << bytesReceived << " bytes" << std::endl;

            if (bytesReceived > 0)
            {
                buffer[bytesReceived] = '\0';
                std::cout << PINK << "--- message content = " << buffer << RESET << std::endl;

                // BUFFER SECTION
                //std::cout << "- BUFFER PROCESSING" << std::endl;
                _clients[i]->addData(std::string(buffer));

                // AUTHENTICATION SECTION                                                                                             // [AJOUT] Authentification début
                if (!_clients[i]->isAuthenticated())
                {
                    std::vector<std::string> messages = _clients[i]->getMessage();

                    if (messages.empty())
                    {
                        auth.requestPassword(_clients[i]);
                    }
                    else
                    {
                        for (size_t j = 0; j < messages.size(); ++j)
                            auth.verifyPassword(_clients[i], messages[j]);
                        _clients[i]->clearBuffer();
                        if (!_clients[i]->isAuthenticated())
                            auth.requestPassword(_clients[i]);
                    }

                    continue;                                                                                                           // Ne traite pas les commandes tant que le client n’est pas authentifié
                }                                                                                                                       // Authentification fin

                // PARSING SECTION
                // parseMessage(std::string(buffer));                                                                                   // [À SUPPRIMER] OUTDATED, j'ai fait mon propre parsing

                //                                                                                                                      // [AJOUT]
                std::vector<std::string> messages = _clients[i]->getMessage();                                                          // Récupère le message
                for (size_t j = 0; j < messages.size(); ++j)
                    cmdHandler.handleCommand(_clients[i], messages[j]);                                                                 // [BRACHEMENT] Lien vers mon code, gestion des commandes
                _clients[i]->clearBuffer();
                printClients();

                // SEND SECTION
                //std::cout << "- SEND RESPONSE" << std::endl;
                // std::string response = "SERVER ECHO = ";
                // response += buffer;
                // response += "\r\n";

                // std::cout << "-- sending to client " << _clients[i]->getFd() << std::endl;
                // int sentBytes = send(_clients[i]->getFd(), response.c_str(), response.length(), 0);
                // std::cout << GREEN << "--- sent " << sentBytes << " bytes" << RESET << std::endl;
            }
            else if (bytesReceived == 0)
            {
                // DISCONNECT SECTION
                std::cout << YELLOW << "- CLIENT DISCONNECT" << RESET << std::endl;
                std::cout << "-- client " << _clients[i]->getFd() << " disconnected cleanly" << std::endl;
                close(_clients[i]->getFd());
                delete (_clients[i]);
                std::cout << "--- client object deleted" << std::endl;

                _clients.erase(_clients.begin() + i);
                std::cout << "---- remaining clients = " << _clients.size() << std::endl;
                break ;
            }
            else
            {
                // ERROR SECTION
                std::cout << RED << "- CLIENT ERROR" << RESET << std::endl;
                std::cout << "-- recv error for client " << _clients[i]->getFd() << std::endl;
                close(_clients[i]->getFd());
                delete (_clients[i]);
                _clients.erase(_clients.begin() + i);
                std::cout << "--- ERROR:client removed and remaining = " << _clients.size() << std::endl;
                break ;
            }
        }
        //std::cout << "-- client check OK" << std::endl;
    }
}

//FIX PORT ERROR
void Server::closePort()
{
    int opt = 1;

    //std::cout << YELLOW << "- CLOSE PORT" << std::endl;

    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        std::cerr << RED << "ERROR: closePort failed" << RESET << std::endl;
    //else
    //    std::cout << GREEN << "--- Port reuse OK" << RESET << std::endl;
}


// ----- Channel functions -----

// Finds a Channel by its name
Channel* Server::getChannelByName(const std::string& name) const
{
    for (size_t i = 0; i < _channels.size(); ++i)
        if (_channels[i]->getName() == name)
            return _channels[i];
    return NULL;
}

// Finds a Client by its name
Client* Server::getClientByNick(const std::string& nick) const
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i]->getNickname() == nick)
            return _clients[i];
    }
    return NULL; // not found
}

// Adds new channel to server
void Server::addChannel(Channel* ch)
{
    _channels.push_back(ch);
}




// ----- Debug functions -----

// Prints every client and their info
void Server::printClients() const
{
    const int fdWidth = 4;
    const int authWidth = 4;
    const int regWidth = 3;
    const int nickWidth = 12;
    const int userWidth = 10;

    std::cout << "==================== CLIENTS ====================" << std::endl;
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