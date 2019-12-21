//
// Created by Alicja Halamska on 29.11.2019.
//

#include <thread>
#include <utility>
#include <functional>
#include "GameManager.h"
#include "../UserManagement/User.h"
using namespace std;



void GameManager::createGame() {
    userA->createWord();
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

void GameManager::endGame(int winnerFd, in_addr ip){
    this->word = "";
    UserA userA1 = UserA(winnerFd, ip);
    this->userA = &userA1;
    this->questionsAnswers.clear();
    createGame();
}

void GameManager::endGameWhenUserALeft() {
    this->word = "";
    auto iterator = users.begin();
    std::advance( iterator, random() % users.size() );
    UserA userA1 = UserA(iterator->first, iterator->second->getIp());
    this->userA = &userA1;
    this->questionsAnswers.clear();
    createGame();
}

void GameManager::questionsLoop() {
    while(true){
        for (auto& user : this->users) {
            if(user.second->getLife() != 0){
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
    string sentence =  question + "->" + response;
    for(auto user : users){
        int fd = user.first;
        MessagesHandler::getInstance().sendMessage(fd, sentence, QA);
    }
}

void GameManager::setWord(const string& word) {
    GameManager::word = word;
}

map<int, User*> GameManager::getUsers(){
    return this->users;
}

void GameManager::addUserA(UserA &userA) {
    printf("Adding user A\n");
    this->userA = &userA;
    UserA* userARef = this->userA;
    thread userAThread([userARef](){
        userARef->runReadingAnswers();
    });
    printf("Sending information about UserType : User A\n");
    MessagesHandler::getInstance().sendMessage(userA.getSocketFd(), "You are gamer A", USER_A);
}

void GameManager::addUser(User& user) {
    users[user.getSocketFd()] = &user;
    User* userRef = &user;
        thread userThread([userRef](){
            userRef->runReadingAnswers();
        });
    userThread.detach();
    if(users.size() == 1){
        MessagesHandler::getInstance().sendMessage(user.getSocketFd(), "", USER_B);
        createGame();
    } else{
        MessagesHandler::getInstance().sendMessage(user.getSocketFd(), "", USER_B);
        addUserToAlreadyBeganGame(user);
    }

}

void GameManager::removeUser(User& user){
    printf("removing %d\n", user.getSocketFd());
    shutdown(user.getSocketFd(), SHUT_RDWR);
    close(user.getSocketFd());
    {
        unique_lock<mutex> lock(clientFdsLock);
        this->users.erase(user.getSocketFd());
    }
}

void GameManager::removeUserA(){
    printf("removing user A");
    shutdown(userA->getSocketFd(), SHUT_RDWR);
    close(userA->getSocketFd());
    {
        unique_lock<mutex> lock(clientFdsLock);
        userA->~UserA();
        endGameWhenUserALeft();
    }
}

void GameManager::searchForAlivePlayers() {
    for (auto& user : this->users) {
        if(user.second->getLife() > 0) return;
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
        sentence = question.first + "->" + question.second;
        MessagesHandler::getInstance().sendMessage(fd, sentence, QA);
    }
    MessagesHandler::getInstance().sendMessage(fd, "", QA_END);

}

void GameManager::addAnswer(const string& question, string answer) {
    this->questionsAnswers[question] = std::move(answer);
}

void GameManager::askQuestion(string question) {
    userA->askQuestion(std::move(question));
}



