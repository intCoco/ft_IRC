#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message
{
    private:
        std::string _command;
        std::vector<std::string> _params;
    
    public:
        Message();
        ~Message();
        Message(const Message& copy);
        Message& operator=(const Message& copy);
    
        bool deleteSpace(const std::string& input);
    
        std::string getCommand() const;
        int getParamCount() const;

        void printParam() const;
};

#endif
