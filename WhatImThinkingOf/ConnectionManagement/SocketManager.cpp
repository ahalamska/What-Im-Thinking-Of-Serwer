#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <mutex>
#include <signal.h>
#include <thread>
#include "../GameManagement/GameManager.h"



using namespace std;

const char *PORT = "8081";
int addingUsersFd;

uint16_t readPort(char *txt);

void setReuseAddr(int sock);

void startListening();

void acceptNewClients();

void configureSocket();

void acceptGamerA();

int main(int argc, char **argv) {
    thread gamesThread([]() {
        GameManager::getInstance().gamesLoop();
    });
    startListening();
    cout<<"Ended listening"<<endl;
    gamesThread.join();
}


uint16_t readPort(const char *txt) {
    char *ptr;
    auto port = strtol(txt, &ptr, 10);
    if (*ptr != 0 || port < 1 || (port > ((1 << 16) - 1))) error(1, 0, "illegal argument %s", txt);
    return port;
}

void setReuseAddr(int sock) {
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (res) error(1, errno, "setsockopt failed");
}

void startListening() {
    configureSocket();
    acceptGamerA();
}

void configureSocket() {
    auto guessingPort = readPort(PORT);

    addingUsersFd = socket(AF_INET, SOCK_STREAM, 0);
    if (addingUsersFd == -1) error(1, errno, "Creating socket failed");

    signal(SIGPIPE, SIG_IGN);
    setReuseAddr(addingUsersFd);

    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short) guessingPort), .sin_addr={INADDR_ANY}};

    int res = bind(addingUsersFd, (sockaddr *) &serverAddr, sizeof(serverAddr));
    if (res) error(1, errno, "bind failed");

    res = listen(addingUsersFd, 1);
    if (res) error(1, errno, "listen failed");
}

void acceptNewClients() {
    while (true) {
        printf("Waiting for new user B\n");
        // prepare placeholders for client address
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);

        // accept new connection
        auto clientFd = accept(addingUsersFd, (sockaddr *) &clientAddr, &clientAddrSize);
        if (clientFd == -1) error(1, errno, "Accepting connection failed");
        User *userRef = new User(clientFd, clientAddr.sin_addr, "", USER_B_TYPE);
        thread([userRef]() {
            GameManager::getInstance().addUser(userRef);
        }).detach();
        // tell who has connected
        printf("New connection from: %s:%hu (fd: %d) \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port),
               clientFd);
    }
}

void acceptGamerA() {
    // prepare placeholders for client address
    sockaddr_in clientAddr{};
    socklen_t clientAddrSize = sizeof(clientAddr);

    // accept new connection
    auto clientFd = accept(addingUsersFd, (sockaddr *) &clientAddr, &clientAddrSize);
    if (clientFd == -1) error(1, errno, "Accepting connection failed");
    printf("UserB A connected from: %s:%hu (fd: %d) \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port),
           clientFd);
    User* userA = new User(clientFd, clientAddr.sin_addr, "", USER_A_TYPE);

    thread addingUserAThread([userA]() {
        GameManager::getInstance().addUserA(userA);
    });
    acceptNewClients();
    addingUserAThread.join();
}