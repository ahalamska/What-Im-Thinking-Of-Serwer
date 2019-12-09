#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <mutex>
#include <unordered_set>
#include <signal.h>
#include "GameManager.h"
#include "User.h"

using namespace std;

int addingUsersFd;
// client sockets
unordered_set<int> clientFds;

// handles SIGINT
void ctrl_c(int);

// sends data to clientFds excluding fd
void sendToAllBut(int fd, char * buffer, int count);

// converts cstring to port
uint16_t readPort(char * txt);

// sets SO_REUSEADDR
void setReuseAddr(int sock);

int main(int argc, char ** argv){
    auto guessingPort = readPort("8081");

    // create socket
    addingUsersFd = socket(AF_INET, SOCK_STREAM, 0);
    if(addingUsersFd == -1) error(1, errno, "socket failed");

    // prevent dead sockets from raising pipe signals on write
    signal(SIGPIPE, SIG_IGN);

    setReuseAddr(addingUsersFd);

    // bind to any address and guessingPort provided in arguments
    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)guessingPort), .sin_addr={INADDR_ANY}};

    int res = bind(addingUsersFd, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) error(1, errno, "bind failed");

    // enter listening mode
    res = listen(addingUsersFd, 1);
    if(res) error(1, errno, "listen failed");

/****************************/

    while(true){
        // prepare placeholders for client address
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);

        // accept new connection
        auto clientFd = accept(addingUsersFd, (sockaddr*) &clientAddr, &clientAddrSize);
        if(clientFd == -1) error(1, errno, "accept failed");

        // add client to all clients set
        {
            unique_lock<mutex> lock(clientFdsLock);
            User user(clientFd, clientAddr.sin_addr, "name");
            GameManager::getInstance().addUser(user);
        }

        // tell who has connected
        printf("new connection from: %s:%hu (fd: %d)\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientFd);

    }

/****************************/
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
