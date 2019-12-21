//
// Created by hala6 on 20.12.2019.
//

#ifndef SK2_MESSAGESHANDLER_H
#define SK2_MESSAGESHANDLER_H

#include <map>
#include <cygwin/socket.h>
#include <sys/socket.h>
#include <iostream>
#include "../GameManagement/GameManager.h"
#include "MessageType.h"
#include "Message.h"


using namespace std;

class MessagesHandler {

private:

    map<MessageType, string> sendMessageTypeValueMap;

    MessagesHandler();

public :

    static MessagesHandler& getInstance()
    {
        static MessagesHandler instance;
        return instance;
    }

    MessagesHandler(MessagesHandler const&) = delete;
    void operator=(MessagesHandler const&)  = delete;

    Message retrieveMessage(string message);

    Message readMessage(int fd);

    void sendMessage(int receiverFd, const string &message, MessageType type);

    string getValue(MessageType type);
};


#endif //SK2_MESSAGESHANDLER_H

