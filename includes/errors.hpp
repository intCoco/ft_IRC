#ifndef ERRORS_HPP
#define ERRORS_HPP

#define ERR_NOSUCHNICK "401 :No such nick/channel\r\n"
#define ERR_NOSUCHCHANNEL ":403 No such channel\r\n"
#define ERR_CANNOTSENDTOCHAN "404 :Cannot send to channel\r\n"
#define ERR_UNKNOWNCOMMAND "421 :Unknown command\r\n"
#define ERR_NONICKNAMEGIVEN "431 :No nickname given\r\n"
#define ERR_ERRONEUSNICKNAME "432 :Erroneous nickname\r\n"
#define ERR_NICKNAMEINUSE "433 :Nickname is already in use\r\n"
#define ERR_USERNOTINCHANNEL ":441 User not in channel\r\n"
#define ERR_NOTREGISTERED "451 :You have not registered\r\n"
#define ERR_NEEDMOREPARAMS "461 :Not enough parameters\r\n"
#define ERR_ALREADYREGISTRED "462 :You may not reregister\r\n"
#define ERR_PASSWDMISMATCH "464 :Password incorrect\r\n"
#define ERR_CHANNELISFULL "471 :Channel is full\r\n"
#define ERR_INVITEONLYCHAN "473 :Cannot join channel (+i)\r\n"
#define ERR_BADCHANNELKEY "475 :Cannot join channel (+k)\r\n"
#define ERR_CHANOPRIVSNEEDED ":482 You're not channel operator\r\n"



#endif