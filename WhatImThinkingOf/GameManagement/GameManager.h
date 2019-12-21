#ifndef SK2_GAMEMANAGER_H
#define SK2_GAMEMANAGER_H

#include <string>
#include <map>
#include <condition_variable>
#include <thread>
#include "../UserManagement/User.h"
#include "../UserManagement/UserA.h"
#include "../ConnectionManagement/MessagesHandler.h"

using namespace std;
static condition_variable CV;
static mutex clientFdsLock;

// forward declaration
class User;

class UserA;

class MessagesHandler;


class GameManager {

public:
    static GameManager& getInstance()
    {
        static GameManager instance;
        return instance;
    }

private:

    string word;

    UserA *userA;

    map<string, string> questionsAnswers;

    map<int, User*> users;

    GameManager(){}

public:

    void addAnswer(const string& answer, string basicString);
    void setWord(const string &word);

    void addUser(User& user);

    void addUserA(UserA& userA);

    GameManager(GameManager const&) = delete;
    void operator=(GameManager const&)  = delete;

    void createGame();

    void questionsLoop();

    bool guessWord(const string &word);

    void resendResponse(const string &question, string response);

    void askQuestion(string question);

    void endGameIfNoOneGuessedAnswer();

    void searchForAlivePlayers();

    void removeUser(User &user);

    void endGameWhenUserALeft();

    map<int, User*> getUsers();

    void addUserToAlreadyBeganGame(const User &user);

    void sendPreviousQuestions(int fd);

    void removeUserA();

    void endGame(int winnerFd, in_addr ip);

};


#endif //SK2_GAMEMANAGER_H
