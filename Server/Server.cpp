//
// Created by zhihui on 12/26/19.
//
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void error(const std::string &msg){
    std::cerr<< msg <<std::endl;
    return;
}

int main(int argc, char *argv[]){

    int sockfd, newsockfd, port;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[256];

    if(argc < 2){
        std::cerr<< "ERROR, no port provided!" << std::endl;
        return -1;
    }

    //create an IPV4 TCP stream socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        error("ERROR opening socket!");

    //clear the server address structure
    std::memset(&serv_addr,0,sizeof(serv_addr));

    port = atoi(argv[1]);

    //set up the server address structure for bind()
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind the socket to port
    if(bind(sockfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR on binding.");

    //listen for incoming connections
    listen(sockfd, 5);
    std::cout <<"Start listening on address: "<<inet_ntoa(serv_addr.sin_addr)<<", port: "<<ntohs(serv_addr.sin_port)<<std::endl;

    clilen = sizeof(cli_addr);

    //accept client's incoming connection and create new socket,write client info into client address structure
    newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr, &clilen);
    if(newsockfd < 0)
        error("ERROR on accept.");

    std::cout << "Received connection from "<<
              inet_ntoa(cli_addr.sin_addr) <<", port: "<< htons(cli_addr.sin_port)<<std::endl;

    //send message to client via new socket
    send(newsockfd, "Hello!", 7, 0);

    std::memset(&buffer, 0 ,sizeof(buffer));

    //get message from client via new socket
    int recv_stat = recv(newsockfd, &buffer, sizeof(buffer), 0);
    if (recv_stat < 0)
        error("ERROR on receiving message from socket.");

    std::cout << "Here is the message: "<< buffer << std::endl;

    close(newsockfd);
    close(sockfd);

    return 0;
}