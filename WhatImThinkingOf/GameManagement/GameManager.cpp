//
// Created by Alicja Halamska on 29.11.2019.
//

#include <thread>
#include <sys/unistd.h>
#include "GameManager.h"

using namespace std;


void GameManager::gamesLoop() {
    gameStart.lock();

    if (this->users.empty()) {
        gameStart.lock();
    }
    createGame();
    cout << "Ended first game!" << endl;
    while (true) {
        while (this->users.empty()) {
            gameStart.lock();
        }
        if (winnerFd == userA->getSocketFd()) {
            cout << "winner : " << winnerFd << endl;
            endGameIfNoOneGuessedAnswer();
            createGame();
        } else if (winnerFd != 0) {
            cout << "winner : " << winnerFd << endl;
            endGame(winnerFd);
            createGame();
        }
    }
}


void GameManager::createGame() {
    cout << "Creating new game... " << endl;
    winnerFd = 0;
    while (word.empty()) {
        sleep(1);
    }
    gameRunning = true;
    thread questionThreadLoc([]() {
        GameManager::getInstance().questionsLoop();
    });
    questionThreadLoc.join();
}

void GameManager::endGameIfNoOneGuessedAnswer() {
    cout << "Ending game UserA won!" << endl;
    this->word = "";
    this->questionsAnswers.clear();
    resendThatUserAWon();
}

void GameManager::endGame(int winnerFd) {
    cout << "Ending Game UserB : " << this->users[winnerFd]->getName() << " won!" << endl;
    this->word = "";
    this->questionsAnswers.clear();
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

/*void GameManager::endGameWhenUserALeft() {
    gameRunning = false;
    questionLoop.lock();
    questionLoop.unlock();

    this->word = "";
    auto iterator = users.begin();
    std::advance(iterator, random() % users.size());
    UserA userA1 = UserA(iterator->first, iterator->second->getIp());
    this->userA = &userA1;
    this->users.erase(userA1.getSocketFd());
    this->questionsAnswers.clear();
    createGame();
}*/

void GameManager::questionsLoop() {
    cout << "Started question Loop" << endl;
    questionLoop.lock();
    while (gameRunning) {
        for (auto &user : this->users) {
            cout << "UserB " << user.second->getName() << " with life: " << user.second->getLife() << " turn!" << endl;
            if (user.second->getLife() > 0 && !user.second->getName().empty()) {
                this->userA->setAnswered(false);
                user.second->askQuestion();
                waitForAnswer();
            } else {
                sleep(1);
            }
        }
    }
    cout << "Finished question loop " << endl;
    endQuestionLoop();
}

bool GameManager::guessWord(const string &word) {
    locale loc;
    string lowerWord;
    for (char i : word)
       lowerWord += tolower(i,loc);
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
            lowerWord += tolower(i,loc);
        this->word = lowerWord;
    }
}

void GameManager::addUserA(User *userA) {
    printf("Adding user A\n");
    this->userA = userA;
    User *userARef = this->userA;
    thread userAThread([userARef]() {
        userARef->runReadingAnswers();
    });
    printf("Sending information about UserType : User A\n");
    MessagesHandler::getInstance().sendMessage(userA->getSocketFd(), "", USER_A);
    userAThread.join();
}

void GameManager::addUser(User *user) {
    printf("Adding user B\n");
    users[user->getSocketFd()] = user;

    thread userThread([user]() {
        user->runReadingAnswers();
    });
    printf("Sending information about UserType : User B\n");

    MessagesHandler::getInstance().sendMessage(user->getSocketFd(), "", USER_B);

    if (users.size() == 1) {
        gameStart.unlock();
    } else {
        while (user->getName().empty()) {
            sleep(1);
        }
        addUserToAlreadyBeganGame(user);
    }
    userThread.join();
    cout << "Ended add function XD " << endl;

}

void GameManager::removeUser(User &user) {
    printf("removing user:  %s\n", user.getName().c_str());
    user.setConnected(false);
    shutdown(user.getSocketFd(), SHUT_RDWR);
    close(user.getSocketFd());
    {
        unique_lock<mutex> lock(clientFdsLock);
        User *userToDelete = this->users[user.getSocketFd()];
        this->users.erase(user.getSocketFd());
        delete userToDelete;
    }
}

void GameManager::removeUserA() {
    printf("removing user A");
    userA->setConnected(false);

    shutdown(userA->getSocketFd(), SHUT_RDWR);
    close(userA->getSocketFd());
    {
        unique_lock<mutex> lock(clientFdsLock);
        userA->~User();
        //endGameWhenUserALeft();
    }
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
        sleep(1);
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



