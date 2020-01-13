//
// Created by Alicja Halamska on 15.12.2019.
//

#include <sys/unistd.h>
#include "MessagesHandler.h"

string END_OF_SENTENCE = "//";
using namespace std;


string MessagesHandler::getValue(MessageType type) {
    return sendMessageTypeValueMap[type];
}

void MessagesHandler::sendMessage(int receiverFd, const string &message, MessageType type) {
    string finalMessage = getValue(type) + message + "//\r";
    char *data = finalMessage.data();
    printf("Sending message: %s\n", data);
    int count = send(receiverFd, data, finalMessage.size(), MSG_WAITALL);
    if (count == -1) {
        GameManager::getInstance().removeUser(receiverFd);
    }

}

void MessagesHandler::sendManyQuestions(int fd, const map<std::string, std::string> &questionsAnswers) {
    for (auto &question : questionsAnswers) {
        sendMessage(fd, question.first + "->" + question.second, QA);
    }
}

list<Message> MessagesHandler::readMessage(int fd) {
    string message;
    int count;
    char buf[255];
    count = read(fd, buf, 255);
    if (count == -1) {
        Message msg;
        msg.type = CLOSE;
        msg.message = "Reading exception";
        list<Message> ms;
        ms.insert(ms.end(), msg);
        return ms;
    }
    for (int i = 0; i < count; ++i) {
        message += buf[i];
    }
    return separateMessage(message);
}

list<Message> MessagesHandler::separateMessage(string message) {
    message = incompleteMessage + message;
    incompleteMessage = "";
    list<string> msgList;
    int pos = message.find(END_OF_SENTENCE);
    while (pos != string::npos) {
        msgList.insert(msgList.end(), message.substr(0, pos));
        message = message.substr(pos + 2, message.size() - pos + 1);
        pos = message.find(END_OF_SENTENCE);
    }
    if (!message.empty()) {
        incompleteMessage = message;
    }
    return retrieveMessages(msgList);
}

list<Message> MessagesHandler::retrieveMessages(list<string> messages) {
    list<Message> finalMessages;
    for (string message : messages) {
        Message finalMessage;
        string delimiterType = "||";
        string type = message
                .substr(0, message.find(delimiterType) + 2);
        int nr = message.size() - (message.find(delimiterType) + 2);
        string msg = message
                .substr(message.find(delimiterType) + 2, nr);
        finalMessage.message = msg;
        for (const auto &possibleType : sendMessageTypeValueMap) {
            if (type == possibleType.second) {
                finalMessage.type = possibleType.first;
            }
        }
        printf("Received %s message: %s \n", getValue(finalMessage.type).c_str(), finalMessage.message.c_str());
        finalMessages.insert(finalMessages.end(), finalMessage);
    }
    return finalMessages;
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




