//
// Created by hala6 on 16.12.2019.
//

#include "UserA.h"
using namespace std;


UserA::UserA(int socket, in_addr ip) : socketFd(socket), ip(ip) {}

void UserA::createWord() {
    cout << "Waiting for word" << endl;
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

void UserA::askQuestion(const string& question) {
    cout<<"Asking question : "<< question << endl;
    MessagesHandler::getInstance().sendMessage(socketFd, question, QUESTION);
}

void UserA::runReadingAnswers() {
    cout<<"Run user A loop"<<endl;
    Message message;
    while (true) {
        message = MessagesHandler::getInstance().readMessage(socketFd);
        switch(message.type){
            case QA:
                cout<<"Received answer : " << message.message << endl;
                resendAnswer(message.message);
                break;
            case CLOSE:
                GameManager::getInstance().removeUserA();
                break;
            case NAME:
                name = message.message;
                cout<<"Added name for user A: "<< name << endl;
                break;
            case NEW_WORD:
                cout<<"Got new word: "<< message.message << endl;
                GameManager::getInstance().setWord(message.message);
                break;
        }
    }
}


int UserA::getSocketFd() {
    return socketFd;
}

void UserA::resendAnswer(const basic_string<char>& message) {
    this->answered = true;
    string delimiter = "->";
    string question = message
            .substr(0, message.find(delimiter));
    string answer = message
            .substr(message.find(delimiter) + delimiter.length(), message.length());
    GameManager::getInstance().saveAnswer(question, answer);
}

void UserA::setAnswered(bool answered) {
    UserA::answered = answered;
}

bool UserA::isAnswered() const {
    return answered;
}
