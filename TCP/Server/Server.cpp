//
// Created by zhihui on 12/26/19.
//
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <netdb.h>
#include <poll.h>

void error(const std::string &msg){
    std::cerr<< msg <<std::endl;
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

//create and bind server socket, start listening
int getServerFD(const char *port){

    addrinfo hint,*result,*rptr;
    int sockfd, addr_code;
    int yes = 1;

    std::memset(&hint, 0 ,sizeof(addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    hint.ai_protocol = 0;
    hint.ai_addr = NULL;
    hint.ai_canonname = NULL;
    hint.ai_next = NULL;

    addr_code = getaddrinfo(NULL, port, &hint, &result);
    if(addr_code != 0)
        addrerror("getaddrinfo",addr_code);

    for(rptr = result; rptr != nullptr; rptr->ai_next){
        sockfd = socket(rptr->ai_family,rptr->ai_socktype,rptr->ai_protocol);
        if(sockfd == -1)
            continue;
        setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
        if(bind(sockfd,rptr->ai_addr,rptr->ai_addrlen) == -1){
            close(sockfd);
            continue;
        }
        break;
    }

    if(rptr == nullptr){
        error("Error creating socket");
    }

    freeaddrinfo(result);

    if(listen(sockfd,10) == -1)
        error("Error on listening");

    return sockfd;
}

//add a new socket fd to the poll fd set
void add_to_fds(const int *newfd, pollfd *fds, int *fd_count, int *fd_size){

    /*check for space, if not enough, double it*/
    if(*fd_count == *fd_size){
        *fd_size *= 2;
        fds = (pollfd*)realloc(fds,sizeof(pollfd) * (*fd_size));
    }
    fds[*fd_count].fd = *newfd;
    fds[*fd_count].events = POLLIN;

    ++(*fd_count);
}

//remove socket fd from poll fd set
void del_from_fds(int i, pollfd *fds, int *fd_count){

    fds[i] = fds[*fd_count - 1];
    --(*fd_count);
}

int main(int argc, char *argv[]){

    int poll_count, newfd;
    int fd_count = 0;
    int fd_size = 10;
    pollfd *fds = (pollfd*)malloc(sizeof(pollfd) * fd_size);

    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_storage);
    char clientIP[INET6_ADDRSTRLEN];
    char buffer[1024];

    if(argc < 2){
        std::cerr<< "Usage: " <<argv[0]<<" port"<< std::endl;
        return -1;
    }

    int serv_fd = getServerFD(argv[1]);

    if(serv_fd == -1)
        error("ERROR creating server socket");

    std::cout << "Listening ..."<< std::endl;
    //add server to set and ready to read
    fds[0].fd = serv_fd;
    fds[0].events = POLLIN;
    ++fd_count;

    while(true){
        poll_count = poll(fds,fd_count,-1);

        if(poll_count == -1)
            error("ERROR on polling");

        for(size_t i = 0; i < fd_count; ++i){
            if(fds[i].revents & POLLIN){ //data ready to be read
                if(fds[i].fd == serv_fd){ //if it's the server, handle new connection

                    newfd = accept(serv_fd,(struct sockaddr*)&client_addr,&client_addr_len);
                    if(newfd == -1)
                        error("ERROR on accepting client");
                    else {
                        add_to_fds(&newfd, fds, &fd_count, &fd_size);
                        if(send(newfd,"Welcome to Azeroth!",256,0) == -1)
                            error("ERROR sending");
                        std::cout << "Pollserver: new connection from "
                                  << inet_ntop(client_addr.ss_family, get_in_addr((sockaddr *) &client_addr), clientIP,
                                               INET6_ADDRSTRLEN) << " socket " << newfd << std::endl;

                    }
                }else{ //if it's client
                    int clientfd = fds[i].fd;
                    int byte = recv(clientfd, buffer, sizeof(buffer),0);
                    if(byte <= 0){
                        if(byte == 0){
                            std::cout << "Pollserver: socket hung up: " << clientfd << std::endl;
                        }else{
                            error("ERROR on receving from socket "+clientfd);
                        }
                        close(clientfd);
                        del_from_fds(i,fds,&fd_count);
                    }else{
                        std::cout << "Received message: "<<buffer<<". From socket: "<<clientfd<<std::endl;
                        if(send(clientfd,"You are not prepared!",256,0) == -1)
                            error("ERROR on sending");
                    }
                }
            }
        }
    }
    return 0;
}