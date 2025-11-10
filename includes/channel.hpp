#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "client.hpp"

class Channel
{
private:
    std::string _name;
    std::vector<Client*> _clients;
	std::vector<Client*> _operators;
    std::vector<Client*> _invited;

    std::string _topic;
    bool _modeI;
    bool _modeT;
    size_t _limit;
    std::string _key;

public:
    Channel();
    Channel(const std::string& name);
    Channel(const Channel& other);
    Channel& operator=(const Channel& other);
    ~Channel();

    const std::string& getName() const;
    const std::vector<Client*>& getClients() const;

    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;

	void addOperator(Client* client);
    bool isOperator(Client* client) const;
	void removeOperator(Client* client);

    void addInvite(Client* client);
    bool isInvited(Client* client) const;
	void removeInvited(Client* client);

    std::string getTopic() const;
    void setTopic(const std::string& topic);

    void setModeI(bool enable);
    bool isModeI() const;
    void setModeT(bool enable);
    bool isModeT() const;

    std::string getKey() const;
    void setKey(const std::string& key);

    void setLimit(int limit);
    size_t getLimit() const;

    bool isEmpty() const;

};

#endif
