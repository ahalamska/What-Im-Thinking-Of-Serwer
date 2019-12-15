//
// Created by Alicja Halamska on 29.11.2019.
//

#include "User.h"

#include <utility>
#include "GameManager.h"

User::User(int socket, in_addr ip) : socketFd(socket), ip(ip), life(BEGINNING_NUMBER_OF_LIFE) {}


int User::getSocketFd() const {
    return socketFd;
}



void User::createWord() {
    string word = askForWord();
    GameManager::getInstance().setWord(word);
}

void User::askQuestion() {
    askForQuestion();
    while (question.empty()) {
        sleep(5);
    }
    send(GameManager::getInstance().getUserASockedFd(), &question, question.length(), MSG_DONTWAIT);
    this->question = "";
}

void User::runReadingFromUserSocket() {
    informAboutUserType(this->socketFd == GameManager::getInstance().getUserASockedFd());
    while (life != 0) {
        string sentence;
        int count;
        do {
            char buf[255];
            count = read(this->socketFd, buf, 255);
            if (count < 1) {
                GameManager::getInstance().removeUser(*this);
                break;
            }
            sentence += buf;
        } while (count == 255);
        if (sentence.find(questionPrefix)) {
            sentence = sentence.substr(questionPrefix.size(), sentence.length() - questionPrefix.size());
            saveQuestion(sentence);
        }
        if (sentence.find(wordPrefix)) {
            sentence = sentence.substr(questionPrefix.size(), sentence.length() - questionPrefix.size());
            guessWord(sentence);
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
        GameManager::getInstance().endGame(this->socketFd);
    }

}

string User::askForWord() {
    send(GameManager::getInstance().getUserASockedFd(), "?word", question.length(), MSG_DONTWAIT);
    string word;
    do{
        int count = read(this->socketFd, &word, 255);
        if (count < 1) {
            GameManager::getInstance().removeUser(*this);
        }
    }while(!word.find(wordPrefix));
    return word.substr(questionPrefix.size(), word.length() - questionPrefix.size());



}

void User::askForQuestion() {
    send(GameManager::getInstance().getUserASockedFd(), "?question", question.length(), MSG_DONTWAIT);
}

int User::getLife() {
    return life;
}

void User::setName(string name) {
    this->name = name;
}

void User::informAboutUserType(bool isGuessingType) {
    string msg = isGuessingType ? "A||//" : "B||//";
    send(this->socketFd, &msg, msg.length(), MSG_DONTWAIT);

}




