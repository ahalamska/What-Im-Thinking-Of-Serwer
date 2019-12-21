//
// Created by Alicja Halamska on 16.12.2019.
//

#ifndef SK2_USER_A_H
#define SK2_USER_A_H

#pragma once
#include <cygwin/in.h>
#include <string>
#include "../ConnectionManagement/MessagesHandler.h"
#include "../GameManagement/GameManager.h"



using namespace std;

class UserA {

private:

    int socketFd;
    in_addr ip;
    string name;


public:
    UserA(int socket, in_addr ip);

    UserA(){};

    void createWord();

    void runReadingAnswers();

    string waitForWord();

    int getSocketFd();

    void askQuestion(string question);

    void sendAnswer(Message message);

    void win();

    void loose();
};


#endif //SK2_USER_A_H
