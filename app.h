#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <openssl/sha.h>

using namespace std;

struct datapackage {
    uint32_t id;
    uint16_t length;
    unsigned char* dp;
};

class App {
    public:
        App(char* domainName, char* port);
        ~App();

        void run();

    private:
        char* hostname;
        char* hostport;
        unsigned char* buffer;

        int sockfd;
        int wp;
        int rp;

        vector<datapackage> dict;

        void creatSocket();
        void download();
        void socketClose();
        void creatFile();
        void sha256();
        void errorLog(const char*);

        int parseBuffer();
};