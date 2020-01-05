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
    send(receiverFd, data, finalMessage.size(), MSG_WAITALL);
    cout<<"Message sent successfully"<<endl;
}

void MessagesHandler::sendManyQuestions(int fd, const map<std::string, std::string>& questionsAnswers) {
    string finalMessage = getValue(QA);
    for (auto& question : questionsAnswers) {
        finalMessage += question.first + "->" + question.second + "//";
        printf("question %s -> %s", question.first.c_str(), question.second.c_str());
    }
    char* data = finalMessage.data();
    printf("Sending message: %s\n end", data);
    send(fd, data, finalMessage.size(), MSG_WAITALL);
    cout<<"Message sent successfully"<<endl;
}


Message MessagesHandler::readMessage(int fd) {
    string sentence;
    int count;
    do {
        char buf[255];
        count = read(fd, buf, 255);
        if(count == -1){
            Message msg;
            msg.type = CLOSE;
            msg.message = "Reading exception";
            return msg;
        }
        sentence += buf;
    } while (count == 255);
    return retrieveMessage(sentence);
}

Message MessagesHandler::retrieveMessage(const string& message) {
    Message finalMessage;
    string delimiterType = "||";
    string delimiterSentence = "//";
    string type = message
            .substr(0, message.find(delimiterType) + 2);
    string msg = message
            .substr(message.find(delimiterType) + 2, message.find(delimiterSentence));
    finalMessage.message = msg;
    for (const auto& possibleType : sendMessageTypeValueMap) {
        if (type == possibleType.second) {
            finalMessage.type = possibleType.first;
        }
    }
    printf("Received %s message: %s \n",getValue(finalMessage.type).c_str(), finalMessage.message.c_str());
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


