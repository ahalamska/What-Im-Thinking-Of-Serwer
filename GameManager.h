#ifndef SK2_GAMEMANAGER_H
#define SK2_GAMEMANAGER_H

#include <string>
#include <map>
#include <condition_variable>
#include "User.h"

using namespace std;
static condition_variable CV;
static mutex clientFdsLock;

// forward declaration
class User;

class GameManager {

public:
    static GameManager& getInstance()
    {
        static GameManager instance;
        return instance;
    }

    void operator()(){questionsLoop();}

private:

    string word;

    int userASockedFd;

    map<string, string> questionsAnswers;

    map<int, User*> users;

    GameManager(){}

public:
    void setWord(const string &word);

    int getUserASockedFd() const;

    void setUserASockedFd(int sockedFd);

    map<string, string> getQuestionsAnswers() const;

    void addUser(User& user);

    User * getUser(int sockedFd);

    GameManager(GameManager const&) = delete;
    void operator=(GameManager const&)  = delete;

    void createGame();

    void questionsLoop();

    bool guessWord(const string &word);

    void resendResponse(const string &question, string response);

    void endGame(int winnerFd);

    void endGameIfNoOneGuessedAnswer();

    void searchForAlivePlayers();

    void removeUser(User &user);

    void endGameWhenUserALeft();

    map<int, User*> getUsers();

    void addUserToAlreadyBeganGame(const User &user);

    void sendPreviousQuestions(int fd);
};


#endif //SK2_GAMEMANAGER_H
