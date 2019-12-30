//
// Created by zhihui on 12/26/19.
//

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    exit(0);
}

void addrerror(const std::string &msg, int code){
    std::cerr<< msg <<" : "<<gai_strerror(code)<<std::endl;
    exit(0);
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {

    int sockfd;
    addrinfo hint, *result, *rptr;
    char addr_str[INET6_ADDRSTRLEN];

    if(argc < 3)
    {
        std::cerr << "Usage "<<argv[0] << " hostname port" <<std::endl;
        return -1;
    }

    std::memset(&hint, 0 ,sizeof(addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    hint.ai_protocol = 0;
    hint.ai_canonname = nullptr;
    hint.ai_addr = nullptr;
    hint.ai_next = nullptr;

    auto addr_code = getaddrinfo(argv[1],argv[2],&hint,&result);
    if(addr_code != 0)
        addrerror("getaddrinfo", addr_code);

    for(rptr = result; rptr != nullptr; rptr->ai_next){
        sockfd = socket(rptr->ai_family,rptr->ai_socktype,rptr->ai_protocol);
        if(sockfd == -1)
            continue;
        if(connect(sockfd,rptr->ai_addr,rptr->ai_addrlen) == -1){
            close(sockfd);
            continue;
        }
        break;
    }

    if(rptr == nullptr){
        error("Error connecting");
    }

    inet_ntop(rptr->ai_family, get_in_addr(rptr->ai_addr), addr_str, INET6_ADDRSTRLEN);
    std::cout << "Connecting to: "<< addr_str <<std::endl;

    freeaddrinfo(result);

    char buffer[256];
    while(true) {
        //clear buffer and get message from server
        std::memset(&buffer,0,sizeof(buffer));

        if(recv(sockfd,&buffer,sizeof(buffer),0) > 0)
            std::cout <<"Server message: "<<buffer<<std::endl;

        //clear buffer and send message to server
        std::memset(&buffer,0,sizeof(buffer));

        std::cout <<"Enter message to send to server: "<<std::endl;
        std::cin.getline(buffer,sizeof(buffer));

        if(std::strcmp(buffer,"exit") == 0)
            break;
        else
            if(send(sockfd,&buffer,sizeof(buffer),0) < 0)
                error("ERROR sending message to server");
    }

    close(sockfd);
}