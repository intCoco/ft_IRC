/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chuchard <chuchard@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 23:07:44 by nihamdan          #+#    #+#             */
/*   Updated: 2025/10/28 01:23:15 by chuchard         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv)
{
    if (argc != 3)
	{
        std::cerr << "Usage: ./ft_irc <port> <password>\n";
        return 1;
    }
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535)
	{
        std::cerr << "Error: invalid port\n";
        return 1;
    }

    try
	{
        Server srv(port, argv[2]);
        srv.run();
    }
	catch (const std::exception& e)
	{
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
