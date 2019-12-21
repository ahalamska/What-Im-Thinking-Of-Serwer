//
// Created by hala6 on 16.12.2019.
//

#include "UserA.h"
using namespace std;


UserA::UserA(int socket, in_addr ip) : socketFd(socket), ip(ip) {}

void UserA::createWord() {
    string word = waitForWord();
    GameManager::getInstance().setWord(word);
}

string UserA::waitForWord() {
    Message message;
    do{
        message = MessagesHandler::getInstance().readMessage(socketFd);
    }while(message.type != NEW_WORD);
    return message.message;
}

void UserA::askQuestion(string question) {
    MessagesHandler::getInstance().sendMessage(socketFd, question, QUESTION);
}

void UserA::runReadingAnswers() {
    cout<<"runned user A loop"<<endl;
    Message message;
    while (true) {
        message = MessagesHandler::getInstance().readMessage(socketFd);
        switch(message.type){
            case QA:
                sendAnswer(message);
                break;
            case CLOSE:
                GameManager::getInstance().removeUserA();
                break;
            case NAME:
                name = message.message;
                break;
        }
    }
}


int UserA::getSocketFd() {
    return socketFd;
}

void UserA::sendAnswer(Message message) {
    string delimiter = "->";
    string question = message.message
            .substr(0, message.message.find(delimiter));
    string answer = message.message
            .substr(message.message.find(delimiter) + delimiter.length(), message.message.length());
    GameManager::getInstance().addAnswer(question, answer);
}
