//
// Created by hala6 on 20.12.2019.
//

#ifndef SK2_MESSAGESHANDLER_H
#define SK2_MESSAGESHANDLER_H

#include <map>
#include <cygwin/socket.h>
#include <sys/socket.h>
#include <iostream>
#include <list>
#include "../GameManagement/GameManager.h"
#include "MessageType.h"
#include "Message.h"


using namespace std;

class MessagesHandler {

private:

    map<MessageType, string> sendMessageTypeValueMap;

    string incompleteMessage;

    MessagesHandler();

public :

    static MessagesHandler& getInstance()
    {
        static MessagesHandler instance;
        return instance;
    }

    MessagesHandler(MessagesHandler const&) = delete;
    void operator=(MessagesHandler const&)  = delete;

    list<Message> retrieveMessages(list<string> message);

    list<Message> readMessage(int fd);

    void sendManyQuestions(int i, const map<std::string, std::string>& map);

    void sendMessage(int receiverFd, const string &message, MessageType type);

    string getValue(MessageType type);

    list<Message> separateMessage(string message);
};


#endif //SK2_MESSAGESHANDLER_H

