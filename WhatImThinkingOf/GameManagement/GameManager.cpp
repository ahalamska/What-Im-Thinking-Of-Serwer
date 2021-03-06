//
// Created by Alicja Halamska on 29.11.2019.
//

#include <thread>
#include <sys/unistd.h>
#include "GameManager.h"

int MAX_USERS_COUNT = 15;
using namespace std;


void GameManager::gamesLoop() {
    cout << "Started game loop!" << endl;
    while (this->users.empty()) {
        gameStart.lock();
        cout << "Unlocked users!" << endl;
    }
    createGame();
    cout << "Ended first game!" << endl;
    while (true) {
        while (this->users.empty()) {
            gameStart.lock();
        }
        if (winnerFd == 0) {
            cout << "user A left game" << endl;
            endGameWhenUserALeft();
            createGame();
        } else if (winnerFd == userA->getSocketFd()) {
            cout << "winner : " << winnerFd << endl;
            endGameIfNoOneGuessedAnswer();
            createGame();
        } else if (winnerFd != 0) {
            cout << "winner : " << winnerFd << endl;
            endGameIfUserBWon(winnerFd);
            createGame();
        }
    }
}


void GameManager::createGame() {
    cout << "Creating new game... " << endl;
    winnerFd = 0;
    while (word.empty()) {
        waitingForWord.lock();
    }
    gameRunning = true;
    GameManager::getInstance().questionsLoop();
}

void GameManager::endGame() {
    this->word = "";
    this->questionsAnswers.clear();
}

void GameManager::endGameIfNoOneGuessedAnswer() {
    cout << "Ending game UserA won!" << endl;
    endGame();
    resendThatUserAWon();
}

void GameManager::endGameIfUserBWon(int winnerFd) {
    cout << "Ending Game UserB : " << this->users[winnerFd]->getName() << " won!" << endl;
    endGame();
    changeUserAtoUserB(winnerFd);
}

void GameManager::changeUserAtoUserB(int winnerFd) {
    userA->setType(USER_B_TYPE);
    users[userA->getSocketFd()] = userA;
    changeWinnerToUserA(winnerFd);
}

void GameManager::changeWinnerToUserA(int winnerFd) {
    User *winner = this->users[winnerFd];
    winner->setType(USER_A_TYPE);
    this->userA = winner;
    this->users.erase(winnerFd);
}


void GameManager::endGameWhenUserALeft() {
    cout << "Ending Game UserA left game " << endl;
    endGame();
    auto iterator = users.begin();
    std::advance(iterator, random() % users.size());
    User *newUserA = this->users[iterator->first];
    newUserA->setType(USER_A_TYPE);
    this->userA = newUserA;
    this->users.erase(newUserA->getSocketFd());
    MessagesHandler::getInstance().sendMessage(userA->getSocketFd(), "UserA left", USER_A);
    for (auto userB : users) {
        MessagesHandler::getInstance().sendMessage(userB.second->getSocketFd(), "UserA left", USER_B);
    }

}

void GameManager::questionsLoop() {
    cout << "Started question Loop" << endl;
    questionLoop.lock();
    while (gameRunning) {
        while (this->users.empty()) {
            cout<<"czekamy na userów"<<endl;
            waitingForUsers.lock();
        }
        for (auto &user : this->users) {
            if(user.second != nullptr || !user.second->isConnected()){
                cout << "UserB " << user.second->getName() << " with life: " << user.second->getLife() << " turn!" << endl;
                if (user.second->getLife() > 0 && !user.second->getName().empty()) {
                    this->userA->setAnswered(false);
                    user.second->askQuestion();
                    waitForAnswer();
                }
            }
            if(user.second == nullptr || !user.second->isConnected()){
                break;
            }
        }

    }
    cout << "Finished question loop " << endl;
    endQuestionLoop();
}

void GameManager::winGame(int socketFd){
    resendGoodGuess(word, socketFd);
    setWinnerFd(socketFd);
    setGameRunning(false);
    for(auto user : users){
        user.second->questionReady.unlock();
    }
    waitingForAnswer.unlock();
}

bool GameManager::guessWord(const string &word) {
    locale loc;
    string lowerWord;
    for (char i : word)
        lowerWord += tolower(i, loc);
    return this->word == lowerWord;
}

void GameManager::resendResponse(const string &question, const string &response) {
    this->questionsAnswers[question] = response;
    string sentence = question + "->" + response;
    for (auto user : users) {
        int fd = user.first;
        MessagesHandler::getInstance().sendMessage(fd, sentence, QA);
    }
}

void GameManager::resendWrongGuess(const string &guess, int guesserFd) {
    this->questionsAnswers[guess] = "false";
    string sentence = guess + "->false";
    for (auto user : users) {
        int fd = user.first;
        if (fd == guesserFd) {
            MessagesHandler::getInstance().sendMessage(fd, sentence, WRONG_GUESS);
        }
        MessagesHandler::getInstance().sendMessage(fd, sentence, QA);
    }
}

void GameManager::resendGoodGuess(const string &guess, int winnerFd) {
    this->questionsAnswers[guess] = "true";
    string sentence = guess + "->true";
    for (auto user : users) {
        int fd = user.first;
        if (fd == winnerFd) {
            MessagesHandler::getInstance().sendMessage(winnerFd, sentence, WIN);
        } else {
            MessagesHandler::getInstance().sendMessage(fd, sentence, LOOSE);
        }
    }
    MessagesHandler::getInstance().sendMessage(this->userA->getSocketFd(), sentence, LOOSE);
}

void GameManager::resendThatUserAWon() {
    for (auto user : users) {
        int fd = user.first;
        MessagesHandler::getInstance().sendMessage(fd, "", LOOSE);
    }
    MessagesHandler::getInstance().sendMessage(this->userA->getSocketFd(), "", WIN);
}

void GameManager::setWord(const string &word) {
    if (this->word.empty()) {
        locale loc;
        string lowerWord;
        for (char i : word)
            lowerWord += tolower(i, loc);
        this->word = lowerWord;
        waitingForWord.unlock();
    }
}

User *GameManager::getUserA() const {
    return userA;
}

void GameManager::addUserA(User *userA) {
    printf("Adding user A\n");
    this->userA = userA;
    thread userAThread([userA]() {
        userA->runReadingAnswers();
    });
    printf("Sending information about UserType : User A\n");
    MessagesHandler::getInstance().sendMessage(userA->getSocketFd(), "", USER_A);
    userAThread.join();
}

void GameManager::addUser(User *user) {
    printf("Adding user B\n");
    thread userThread([user]() {
        user->runReadingAnswers();
    });
    printf("Sending information about UserType : Guesser\n");

    MessagesHandler::getInstance().sendMessage(user->getSocketFd(), "", USER_B);

    while (user->getName().empty()) {
        waitingForName.lock();
        if (!user->isConnected()) {
            return;
        }
    }

    while(users.size() == MAX_USERS_COUNT){
        MessagesHandler::getInstance().sendMessage(user->getSocketFd(), "", WAIT);
        maxUsers.lock();
    }
    MessagesHandler::getInstance().sendMessage(user->getSocketFd(), "", GAME_BEGAN);


    users[user->getSocketFd()] = user;

    waitingForUsers.unlock();

    if (users.size() == 1) {
        gameStart.unlock();
    } else {
        addUserToAlreadyBeganGame(user);
    }
    userThread.join();
    cout << "Ended add function" << endl;
}

void GameManager::removeUser(User *user) {
    printf("removing user:  %s\n", user->getName().c_str());
    this->users.erase(user->getSocketFd());

    waitingForName.unlock();
    user->setConnected(false);

    shutdown(user->getSocketFd(), SHUT_RDWR);
    close(user->getSocketFd());
    delete user;
    maxUsers.unlock();
}

void GameManager::removeUser(int fd) {
    if(this->users.find(fd) != users.end()) {
        removeUser(users[fd]);
    }
}

void GameManager::removeUserA() {
    printf("removing user A\n");
    userA->setConnected(false);
    waitingForAnswer.unlock();
    gameRunning = false;
    waitingForWord.unlock();

    waitingForUsers.unlock();

    shutdown(userA->getSocketFd(), SHUT_RDWR);
    close(userA->getSocketFd());

    questionLoop.lock();
    questionLoop.unlock();
    delete userA;
    this->userA = nullptr;

}

void GameManager::searchForAlivePlayers() {
    for (auto &user : this->users) {
        if (user.second->getLife() > 0) return;
    }
    cout << "Haven't found alive players" << endl;
    gameRunning = false;
    setWinnerFd(userA->getSocketFd());
    questionLoop.lock();
    questionLoop.unlock();
    endGameIfNoOneGuessedAnswer();
}

void GameManager::addUserToAlreadyBeganGame(User *user) {
    sendPreviousQuestions(user->getSocketFd());
}

void GameManager::sendPreviousQuestions(int fd) {
    MessagesHandler::getInstance().sendManyQuestions(fd, questionsAnswers);
}

void GameManager::saveAnswer(const string &question, const string &answer) {
    this->questionsAnswers[question] = answer;
    this->resendResponse(question, answer);

}

void GameManager::askQuestion(const string &question) {
    userA->askQuestion(question);
}

void GameManager::waitForAnswer() {
    while (!userA->isAnswered() && userA->isConnected() && GameManager::getInstance().isGameRunning()) {
        waitingForAnswer.lock();
    }
}

void GameManager::endQuestionLoop() {
    for (auto user : users) {
        user.second->resetUser();
    }
    this->userA->setAnswered(true);
    questionLoop.unlock();
}

bool GameManager::isGameRunning() const {
    return gameRunning;
}

map<int, User *> GameManager::getUsers() const {
    return users;
}

void GameManager::setWinnerFd(int winnerFd) {
    GameManager::winnerFd = winnerFd;
}

void GameManager::setGameRunning(bool gameRunning) {
    GameManager::gameRunning = gameRunning;
}



