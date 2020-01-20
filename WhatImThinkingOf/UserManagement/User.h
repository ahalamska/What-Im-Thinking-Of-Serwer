//
// Created by hala6 on 06.01.2020.
//

#ifndef SK2_USER_H
#define SK2_USER_H


#include <cygwin/in.h>
#include <string>
#include <list>
#include <mutex>
#include "../ConnectionManagement/Message.h"

using namespace std;
class MessagesHandler;
const string USER_A_TYPE = "UserA";
const string USER_B_TYPE = "UserB";
const int BEGINNING_NUMBER_OF_LIFE = 3;



class User {


protected:
    string type;
    int socketFd;
    in_addr ip{};
    string name;
    int life;
    string question;
    bool connected = true;
    bool answered = true;

public:
    timed_mutex questionReady;

    User(int fd, in_addr ip, string name, string type);
    const string &getType() const;
    void setType(const string &type);
    const in_addr &getIp() const;
    const string &getName() const;
    bool isConnected();
    int getSocketFd();
    void setConnected(bool connected);
    void addMessage(Message message);

    void runReadingAnswers();

    void setAnswered(bool answered);

    bool isAnswered() const;

    void resendAnswer(const basic_string<char> &message);

    void askQuestion(const string &question);

    void askQuestion();

    void askForQuestion();

    void guessWord(string word);

    void saveQuestion(string question);

    void resetUser();


    int getLife();

    mutex readingLoopEnded;
};


#endif //SK2_USER_H
