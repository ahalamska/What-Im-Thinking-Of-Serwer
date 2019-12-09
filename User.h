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
#include "GameManager.h"

using namespace std;

static int BEGINNING_NUMBER_OF_LIFE;
static string questionPrefix = "question::";

static string wordPrefix = "word::";

class User {

public:
    User(){}

    User(int socket, in_addr ip, string name);

    void operator()(){runReadingFromUserSocket();}

    int getSocketFd() const;

private:
    int socketFd;
    in_addr ip;
    string name;
    int life;
    string question;

    void askForQuestion();
public:

    void createWord();

    void saveQuestion(string question);

    void askQuestion();

    void guessWord(string word);

    string askForWord();

    void removeUser();

    void runReadingFromUserSocket();

    int getLife();
};


#endif //SK2_USER_H




