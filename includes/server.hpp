/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chuchard <chuchard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 23:06:30 by nihamdan          #+#    #+#             */
/*   Updated: 2025/10/28 06:59:26 by chuchard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>

class Client;
class Channel;

class Server
{
	private:
    	int _serverFd;
    	int _port;
    	std::vector<Client*> _clients;
    	std::vector<Channel*> _channels;
    	std::string _password;

    	void makeServerSocket();
    	void addNewClient();
    	void dropClient(int fd);
    	void handleReadable(int fd);

	public:
    	Server(int port, const std::string& password);
    	~Server();

    	void run();

    	const std::vector<Client*>& getClients() const;
		const std::string getPassword() const;
    	Client* getClientByNick(const std::string& nickname) const;
    	Channel* getChannelByName(const std::string& name) const;
    	void addChannel(Channel* ch);
    	void printClients() const;
		void printChannelInfo(const Channel* ch) const;

};

#endif

