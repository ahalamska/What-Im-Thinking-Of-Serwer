//
// Created by Alicja Halamska on 29.11.2019.
//

#ifndef SK2_USER_H
#define SK2_USER_H

#include <string>
#include <utility>
#include <cygwin/in.h>
#include <unistd.h>
#include <mutex>
#include "../GameManagement/GameManager.h"
#include "../ConnectionManagement/MessageType.h"
#include "../ConnectionManagement/Message.h"
#include "../ConnectionManagement/MessagesHandler.h"

#include <utility>

class MessagesHandler;

using namespace std;

static int BEGINNING_NUMBER_OF_LIFE = 3;

class User {

public:
    User(){}

    User(int socket, in_addr ip);

    void operator()(){ runReadingAnswers();}

    int getSocketFd() const;

    const in_addr &getIp() const;

private:
    int socketFd;
    in_addr ip;
    string name;
    int life;
    string question;

public:

    void saveQuestion(string question);

    void askQuestion();

    void guessWord(string word);

    void runReadingAnswers();

    int getLife();

    void askForQuestion();

    void win();

    void loose();
};


#endif //SK2_USER_H




