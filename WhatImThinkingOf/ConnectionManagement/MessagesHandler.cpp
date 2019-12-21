//
// Created by Alicja Halamska on 15.12.2019.
//

#include "MessagesHandler.h"
using namespace std;


string MessagesHandler::getValue(MessageType type) {
    return sendMessageTypeValueMap[type];
}

void MessagesHandler::sendMessage(int receiverFd, const string &message, MessageType type) {
    string finalMessage = getValue(type) + message + "//\r";
    char* data = finalMessage.data();
    printf("Sending message: %s\n", data);
    send(receiverFd, data, finalMessage.size(), MSG_DONTWAIT);
    cout<<"Message sent successfully"<<endl;
}

Message MessagesHandler::readMessage(int fd) {
    printf("Waiting for message\n");
    string sentence;
    int count;
    do {
        char buf[255];
        count = read(fd, buf, 255);
        sentence += buf;
        printf("Received %d bytes: %s \n", count, sentence.c_str());
    } while (count == 255);
    return retrieveMessage(sentence);
}

Message MessagesHandler::retrieveMessage(const string& message) {
    Message finalMessage;
    string delimiter = "||";
    string type = message
            .substr(0, message.find(delimiter) + 2);
    string answer = message
            .substr(message.find(delimiter) + 2, message.length());
    finalMessage.message = message;
    for (const auto& possibleType : sendMessageTypeValueMap) {
        if (type == possibleType.second) {
            finalMessage.type = possibleType.first;
        }
    }
    printf("Received %o message: %s \n",finalMessage.type, finalMessage.message.c_str());
    return finalMessage;
}

MessagesHandler::MessagesHandler() {
    sendMessageTypeValueMap[USER_A] = "A||";
    sendMessageTypeValueMap[USER_B] = "B||";
    sendMessageTypeValueMap[WIN] = "W||";
    sendMessageTypeValueMap[LOOSE] = "L||";
    sendMessageTypeValueMap[NEW_WORD] = "NW||";
    sendMessageTypeValueMap[QA] = "QA||";
    sendMessageTypeValueMap[QA_END] = "QAE||";
    sendMessageTypeValueMap[QUESTION] = "Q||";
    sendMessageTypeValueMap[ASK_QUESTION] = "AQ||";
    sendMessageTypeValueMap[GAME_BEGAN] = "GB||";
    sendMessageTypeValueMap[WRONG_GUESS] = "WG||";
    sendMessageTypeValueMap[CLOSE] = "CL||";
    sendMessageTypeValueMap[NAME] = "N||";
    sendMessageTypeValueMap[GUESS] = "G||";
}