#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <mutex>
#include <signal.h>
#include "GameManager.h"
#include "User.h"

using namespace std;

int addingUsersFd;

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

void startListening();

void acceptNewClients();

void configureSocket();

int main(int argc, char ** argv){
    startListening();

}



uint16_t readPort(char * txt){
    char * ptr;
    auto port = strtol(txt, &ptr, 10);
    if(*ptr!=0 || port<1 || (port>((1<<16)-1))) error(1,0,"illegal argument %s", txt);
    return port;
}

void setReuseAddr(int sock) {
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (res) error(1, errno, "setsockopt failed");
}

void startListening(){
    configureSocket();
    acceptNewClients();
}

void configureSocket(){
    auto guessingPort = readPort("8080");

    addingUsersFd = socket(AF_INET, SOCK_STREAM, 0);
    if(addingUsersFd == -1) error(1, errno, "Creating socket failed");

    signal(SIGPIPE, SIG_IGN);
    setReuseAddr(addingUsersFd);

    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)guessingPort), .sin_addr={INADDR_ANY}};

    int res = bind(addingUsersFd, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) error(1, errno, "bind failed");

    res = listen(addingUsersFd, 1);
    if(res) error(1, errno, "listen failed");
}

void acceptNewClients(){
    while(true){
        // prepare placeholders for client address
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);

        // accept new connection
        auto clientFd = accept(addingUsersFd, (sockaddr*) &clientAddr, &clientAddrSize);
        if(clientFd == -1) error(1, errno, "Accepting connection failed");

        // add client to all clients set
        {
            unique_lock<mutex> lock(clientFdsLock);
            User user(clientFd, clientAddr.sin_addr);
            GameManager::getInstance().addUser(user);
        }

        // tell who has connected
        printf("New connection from: %s:%hu (fd: %d) \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientFd);
    }
}
