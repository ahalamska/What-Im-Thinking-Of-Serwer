//
// Created by hala6 on 06.01.2020.
//

#include <iostream>
#include <sys/unistd.h>
#include "User.h"
#include "../GameManagement/GameManager.h"

using namespace std;

bool User::isConnected() {
    return connected;
}

const in_addr &User::getIp() const {
    return ip;
}

const string &User::getName() const {
    return name;
}

int User::getSocketFd() {
    return socketFd;
}

void User::setConnected(bool connected) {
    this->connected = connected;
}

void User::setAnswered(bool answered) {
    this->answered = answered;
}

bool User::isAnswered() const {
    return answered;
}

int User::getLife() {
    return life;
}

User::User(int fd, in_addr ip, string name, string type) {
    this->socketFd = fd;
    this->ip = ip;
    this->name = std::move(name);
    this->life = BEGINNING_NUMBER_OF_LIFE;
    this->question = "";
    this->type = type;
}

void User::resendAnswer(const basic_string<char> &message) {
    if(GameManager::getInstance().guessWord("")) return;
    this->answered = true;
    string delimiter = "->";
    string questionToResend = message
            .substr(0, message.find(delimiter));
    string answerToResend = message
            .substr(message.find(delimiter) + delimiter.length(), message.length());
    GameManager::getInstance().saveAnswer(questionToResend, answerToResend);
    GameManager::getInstance().waitingForAnswer.unlock();

}

void User::askQuestion(const string &question) {
    cout << "Asking question : " << question << endl;
    MessagesHandler::getInstance().sendMessage(socketFd, question, QUESTION);
}

void User::askForQuestion() {
    MessagesHandler::getInstance().sendMessage(socketFd, "", ASK_QUESTION);
}

void User::askQuestion() {
    using Ms = std::chrono::milliseconds;
    while (question.empty()) {
        if (!connected || !GameManager::getInstance().isGameRunning()) {
            return;
        }
        askForQuestion();
        questionReady.try_lock_for(Ms(5000));
    }
    if(GameManager::getInstance().isGameRunning()) {
        GameManager::getInstance().askQuestion(question);
    }
    this->question = "";
}

void User::guessWord(string word) {
    if(GameManager::getInstance().guessWord("")){
        return;
    }
    bool response = GameManager::getInstance().guessWord(word);
    if (!response) {
        GameManager::getInstance().resendWrongGuess(word, socketFd);
        this->life--;
        GameManager::getInstance().searchForAlivePlayers();
    } else {
        GameManager::getInstance().winGame(this->socketFd);
    }
}

void User::runReadingAnswers() {
    cout << "Run user "<< this->type <<" loop "<< endl;
    list<Message> messages;
    readingLoopEnded.lock();
    while (connected) {
        messages = MessagesHandler::getInstance().readMessage(socketFd);
        for(const Message& message: messages) {
            if (type == USER_A_TYPE) {
                switch (message.type) {
                    case QA:
                        cout << "Received answer : " << message.message << endl;
                        resendAnswer(message.message);
                        break;
                    case CLOSE:
                        GameManager::getInstance().removeUserA();
                        break;
                    case NAME:
                        name = message.message;
                        cout << "Added name for user A: " << name << endl;
                        break;
                    case NEW_WORD:
                        cout << "Got new word: " << message.message << endl;
                        GameManager::getInstance().setWord(message.message);
                        break;
                }
            } else if (type == USER_B_TYPE) {
                switch (message.type) {
                    case GUESS:
                        cout << "Received guess form " << name << endl;
                        guessWord(message.message);
                        break;
                    case QUESTION:
                        cout << "Received question form " << name << endl;
                        saveQuestion(message.message);
                        questionReady.unlock();
                        break;
                    case NAME:
                        name = message.message;
                        cout << "Added name for user B : " << name << endl;
                        GameManager::getInstance().waitingForName.unlock();
                        break;
                    case EMPTY:
                        break;
                    case CLOSE:
                        GameManager::getInstance().removeUser(this);
                }
            }
        }
    }
    cout << "Finished reading loop of user "<< endl;
    readingLoopEnded.unlock();
}

void User::saveQuestion(string question) {
    if (this->question.empty()) {
        this->question = question;
    }
}

void User::resetUser() {
    this->question = "";
    this->life = BEGINNING_NUMBER_OF_LIFE;
}

void User::setType(const string &type) {
    this->type = type;
}



