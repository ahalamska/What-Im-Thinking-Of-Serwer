//
// Created by Alicja Halamska on 15.12.2019.
//


#include <map>
#include <cygwin/socket.h>
#include <sys/socket.h>
#include <iostream>

using namespace std;

enum SendMessageType{
    USER_A,
    USER_B,
    WIN,
    LOOSE,
    NEW_WORD,
    QA,
    QUESTION,
    ASK_QUESTION,
    GAME_BEGAN,
    WRONG_GUESS};

class MassagesHandler {

    map<SendMessageType, string> SendMessageTypeValueMap;

public:
    MassagesHandler(){
        SendMessageTypeValueMap[USER_A] = "A||";
        SendMessageTypeValueMap[USER_B] = "B||";
        SendMessageTypeValueMap[WIN] = "W||";
        SendMessageTypeValueMap[LOOSE] = "L||";
        SendMessageTypeValueMap[NEW_WORD] = "NW||";
        SendMessageTypeValueMap[QA] = "QA||";
        SendMessageTypeValueMap[QUESTION] = "Q||";
        SendMessageTypeValueMap[ASK_QUESTION] = "AQ||";
        SendMessageTypeValueMap[GAME_BEGAN] = "GB||";
        SendMessageTypeValueMap[WRONG_GUESS] = "WG||";
    }


    string getValue(SendMessageType type) {
        return SendMessageTypeValueMap[type];
    }

    void sendMessage(int receiverFd, const string& message, SendMessageType type){
        string finalMessage = getValue(type) + message + "//\r";
        int length = finalMessage.length();
        cout<<endl<<finalMessage<<endl;
        send(receiverFd, finalMessage.data(), 10, MSG_DONTWAIT);
    }
};