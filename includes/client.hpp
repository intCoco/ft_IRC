/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chuchard <chuchard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 23:06:45 by nihamdan          #+#    #+#             */
/*   Updated: 2025/10/28 18:12:32 by chuchard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client
{
	private:
    	int         _fd;
    	std::string _nickname;
    	std::string _username;
		std::string _realname;
    	std::string _bufferIn;
    	bool        _authenticated;
    	bool        _registered;

	public:
    	Client(int fd);
    	~Client();

    	int getFd() const;

    	void setNickname(const std::string& nick);
    	void setUsername(const std::string& user);
		void setRealname(const std::string& name);

    	const std::string& getNickname() const;
    	const std::string& getUsername() const;
    	const std::string& getRealname() const;

    	void setAuthenticated(bool ok);
    	bool isAuthenticated() const;

    	void setRegistered(bool ok);
    	bool isRegistered() const;

    	void appendToBuffer(const char* data, size_t len);
    	bool extractLine(std::string& out);

    	std::vector<std::string> getMessage();
};

#endif
