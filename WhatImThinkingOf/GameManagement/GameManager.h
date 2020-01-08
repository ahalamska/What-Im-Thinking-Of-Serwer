#ifndef SK2_GAMEMANAGER_H
#define SK2_GAMEMANAGER_H

#include <string>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "../ConnectionManagement/MessagesHandler.h"
#include "../UserManagement/User.h"

using namespace std;
static condition_variable CV;
static mutex clientFdsLock;

// forward declaration
class User;

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

    mutex questionLoop;

    condition_variable questionLoopRunning;

    User* userA;

    map<string, string> questionsAnswers;

    map<int, User*> users;

    GameManager(){}

    bool gameRunning;

public:
    bool isGameRunning() const;

    void saveAnswer(const string& answer, const string& basicString);

    void setWord(const string &word);

    void addUser(User* user);

    void addUserA(User* userA);

    GameManager(GameManager const&) = delete;

    void operator=(GameManager const&)  = delete;

    void createGame();

    void questionsLoop();

    bool guessWord(const string &word);

    void resendResponse(const string &question, const string& response);

    void askQuestion(const string& question);

   void endGameIfNoOneGuessedAnswer();

    void searchForAlivePlayers();

    void removeUser(User &user);

    void endGameWhenUserALeft();

    void addUserToAlreadyBeganGame(User* user);

    void sendPreviousQuestions(int fd);

    void removeUserA();

    void endGame(int winnerFd);

    void waitForAnswer();

    void resendWrongGuess(const string &guess);

    void resendGoodGuess(const string &guess, int fd);

    void endQuestionLoop();

    void changeUserAtoUserB(int i);

    void changeWinnerToUserA(int fd);

    map<int, User *> getUsers() const;

};


#endif //SK2_GAMEMANAGER_H
