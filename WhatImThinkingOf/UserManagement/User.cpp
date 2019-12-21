//
// Created by Alicja Halamska on 29.11.2019.
//

#include "User.h"


User::User(int socket, in_addr ip) : socketFd(socket), ip(ip), life(BEGINNING_NUMBER_OF_LIFE) {}


int User::getSocketFd() const {
    return socketFd;
}





void User::askQuestion() {
    askForQuestion();
    while (question.empty()) {
        sleep(5);
    }
    GameManager::getInstance().askQuestion(question);
    this->question = "";
}

void User::runReadingAnswers() {
    Message message;
    while (life != 0) {
        message = MessagesHandler::getInstance().readMessage(socketFd);
        switch(message.type){
            case GUESS:
                guessWord(message.message);
                break;
            case QUESTION:
                saveQuestion(question);
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


void User::saveQuestion(string question){
    if (question.empty()) {
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




