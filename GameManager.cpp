//
// Created by Alicja Halamska on 29.11.2019.
//

#include <thread>
#include <utility>
#include <functional>
#include "GameManager.h"
#include "User.h"
using namespace std;

void GameManager::createGame() {
    this->users[this->userASockedFd]->createWord();
    for(auto user : this->getUsers()){
        thread userThread([&user](){
            user.second->runReadingFromUserSocket();
        });
        userThread.join();
    }
    thread questionThread([](){
        GameManager::getInstance().questionsLoop();
    });
    questionThread.join();
}

void GameManager::endGameIfNoOneGuessedAnswer(){
    this->word = "";
    this->questionsAnswers.clear();
    createGame();
}

void GameManager::endGame(int winnerFd){
    this->word = "";
    this->userASockedFd = winnerFd;
    this->questionsAnswers.clear();
    createGame();
}

void GameManager::endGameWhenUserALeft() {
    this->word = "";
    auto iterator = users.begin();
    std::advance( iterator, random() % users.size() );
    this->userASockedFd = iterator->first;
    this->questionsAnswers.clear();
    createGame();
}

void GameManager::questionsLoop() {
    while(true){
        for (auto& user : this->users) {
            if(user.first != this->userASockedFd && user.second->getLife() != 0){
                user.second->askQuestion();
            }
        }
    }
}

bool GameManager::guessWord(const string& word) {
        return this->word == word;
}

void GameManager::resendResponse(const string& question, string response) {
    this->questionsAnswers[question] = std::move(response);
    string sentence =  question + " -> " + response;
    for(auto user : users){
        int fd = user.first;
        messagesHandler.sendMessage(fd, sentence, QA);
    }
}

void GameManager::setWord(const string& word) {
    GameManager::word = word;
}

map<string, string> GameManager::getQuestionsAnswers() const {
    return this->questionsAnswers;
}

map<int, User*> GameManager::getUsers(){
    return this->users;
}

void GameManager::addUser(User& user) {
    users[user.getSocketFd()] = &user;
    if(users.size() == 1){
        setUserASockedFd(user.getSocketFd());
        messagesHandler.sendMessage(user.getSocketFd(), "", USER_A);
    }
    else if(users.size() == 2){
        messagesHandler.sendMessage(user.getSocketFd(), "", USER_B);
        createGame();
    } else{
        messagesHandler.sendMessage(user.getSocketFd(), "", USER_B);
        addUserToAlreadyBeganGame(user);
    }

}

int GameManager::getUserASockedFd() const {
    return this->userASockedFd;
}

void GameManager::setUserASockedFd(int userAIp) {
    this->userASockedFd = userAIp;
}

void GameManager::removeUser(User& user){
    printf("removing %d\n", user.getSocketFd());
    shutdown(user.getSocketFd(), SHUT_RDWR);
    close(user.getSocketFd());
    {
        unique_lock<mutex> lock(clientFdsLock);
        this->users.erase(user.getSocketFd());
        if(user.getSocketFd() == userASockedFd){
            endGameWhenUserALeft();
        }
    }
}

User* GameManager::getUser(int sockedFd) {
    return this->users[sockedFd];
}

void GameManager::searchForAlivePlayers() {
    for (auto& user : this->users) {
        if(user.second->getLife() > 0 && user.first != userASockedFd) return;
    }
    endGameIfNoOneGuessedAnswer();
}

void GameManager::addUserToAlreadyBeganGame(const User &user) {
    thread userThread((User()));
    userThread.join();
    sendPreviousQuestions(user.getSocketFd());
}

void GameManager::sendPreviousQuestions(int fd) {
    string sentence;
    for (auto& question : questionsAnswers) {
        sentence = question.first + " -> " + question.second;
        messagesHandler.sendMessage(fd, sentence, QA);
    }
}

