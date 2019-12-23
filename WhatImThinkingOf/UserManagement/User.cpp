//
// Created by Alicja Halamska on 29.11.2019.
//

#include "User.h"

const string &User::getName() const {
    return name;
}

User::User(int socket, in_addr ip) : socketFd(socket), ip(ip), life(BEGINNING_NUMBER_OF_LIFE) {}


int User::getSocketFd() const {
    return socketFd;
}

void User::askQuestion() {
    if(question.empty()){
        askForQuestion();
        while (question.empty()) {
            askForQuestion();
            sleep(10);
        }
    }
    GameManager::getInstance().askQuestion(question);
    this->question = "";
}

void User::runReadingAnswers() {
    cout<<"Run user B loop \n";
    Message message;
    while (life != 0) {
        cout<<"Waiting for message : User B -> "<< socketFd << endl;
        message = MessagesHandler::getInstance().readMessage(socketFd);
        switch(message.type){
            case GUESS:
                cout<<"Received guess form "<< name << endl;
                guessWord(message.message);
                break;
            case QUESTION:
                cout<<"Received question form "<< name << endl;
                saveQuestion(message.message);
                break;
            case CLOSE:
                GameManager::getInstance().removeUser(*this);
                break;
            case NAME:
                name = message.message;
                cout<<"Added name for user B "<< name << endl;
                break;
        }
    }
}


void User::saveQuestion(string question){
    if (this->question.empty()) {
        this->question = question;
    }
}

void User::guessWord(string word) {
    bool response = GameManager::getInstance().guessWord(word);
    GameManager::getInstance().resendResponse(word, response ? "true" : "false");
    if (!response) {
        this->life--;
        GameManager::getInstance().searchForAlivePlayers();
    }
    else{
        GameManager::getInstance().endGame(this->socketFd, this->ip);
    }
}

int User::getLife() {
    return life;
}

const in_addr &User::getIp() const {
    return ip;
}

void User::askForQuestion() {
    MessagesHandler::getInstance().sendMessage(socketFd, "", ASK_QUESTION);
}




