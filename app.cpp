#include "app.h"

const int MAX_BUFFER_SIZE = 10 * 1024*1024;

template <class datapackage>
bool idsort(datapackage a, datapackage b) {
    return a.id < b.id;
}

App::App(char* domainName, char* port) {
    hostname = domainName;
    hostport = port;
    buffer = (unsigned char*)malloc(MAX_BUFFER_SIZE);
}

App::~App() {
    free(buffer);
}

void App::errorLog(const char* message) {
    ofstream myfile;
    myfile.open ("error.log", ios_base::app);
    myfile << message <<"\n";
    myfile.close();
}

void App::run() {
    creatSocket();
    if (sockfd == 0) { 
        return ;
    }
    download();
    if (parseBuffer() == 0) {
        creatFile();
    }
}

void App::creatSocket() {
    struct addrinfo hints = {0}, *addrs, *p;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
   
    hints.ai_protocol = IPPROTO_TCP;

    const int status = getaddrinfo(hostname, hostport, &hints, &addrs);

    if (status != 0) {
        errorLog(gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    //use the first avalible address
    for(p = addrs; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            errorLog("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            errorLog("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        errorLog("client: failed to connect\n");
    }
    //release getaddrinfo space
    freeaddrinfo(addrs);
    printf("connect\n");
}
void App::download() {
    int count = 0;
    size_t len = 0;
    wp = 0;
    printf("fetching...\n");
    while ((len = recv(sockfd, buffer +wp, MAX_BUFFER_SIZE -wp, 0)) > 0) {
        wp += len;
        count += 1;
    }
    printf("total packge: %d , total size: %d\n",wp, count);
    socketClose();
}
void App::socketClose() {
    close(sockfd);
    printf("disconnect\n");
}
int App::parseBuffer() {
    rp = 0;
    unsigned char* copy = buffer;
    printf("checking packages....\n");
    while (rp < wp) {  
        uint32_t ID = be32toh(*(uint32_t*)copy);
        uint16_t length = be16toh(*(uint16_t*)(copy+4));

        dict.push_back({ID, length, copy+6});
        rp += length+6;
        copy += length+6;
    }
    if (rp != wp) {
        errorLog("recv data not correct!!");
        exit(1);
    }

    if (dict.size() == 0) {
        errorLog("Didn't receive correct data!!");
        exit(1);
    }
    printf("Total file block is : %lu, file size is : %d\n", dict.size(), rp);
    //reorder all package
    sort(dict.begin(), dict.end(), idsort<datapackage>);
    sha256();
    return 0;
}
void App::creatFile() {
    //write package to file
    vector<datapackage>  :: iterator iter = dict.begin();
    printf("Creating file...\n");
    FILE* pFile = fopen("download", "wb");
    unsigned fid = -1;
    int fsize = 0;
    for(int ix = 0; iter != dict.end(); ++iter, ++ix) {
        if (fid == iter->id) {
            printf("duplicate file %u", fid);
            continue;
        }
        fid = iter->id;
        fwrite((const void*)iter->dp, 1, iter->length, pFile);
        fsize += iter->length;
    }
    fclose(pFile);
    printf("file \"download\" has download successfuly!\n");
    printf("File final size is: %d\n", fsize);
}

//sha256 checksum
void App::sha256() {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    vector<datapackage> :: iterator iter = dict.begin();
    for(int ix = 0; iter != dict.end(); ++iter, ++ix) {
        SHA256_Update(&sha256, iter->dp, iter->length);
    }
    unsigned char result[SHA256_DIGEST_LENGTH];
    SHA256_Final(result, &sha256);
    printf("SHA256 check sum is: ");
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", result[i]);
    }
    printf("\n");
}