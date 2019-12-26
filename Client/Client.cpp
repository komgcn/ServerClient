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

void error(const std::string &msg) {
    std::cerr << msg << std::endl;
    exit(0);
}

int main(int argc, char *argv[]) {

    int sockfd, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if(argc < 3)
    {
        std::cerr << "Usage "<<argv[0] << " hostname port" <<std::endl;
        return -1;
    }

    port = atoi(argv[2]);

    //create socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0)
        error("ERROR creating socket.");

    //get server info
    server = gethostbyname(argv[1]);
    if (server == nullptr){
        std::cerr << "ERROR, No such host"<<std::endl;
        return -1;
    }

    std::cout << "connecting to server address = " << (server->h_addr_list[0][0] & 0xff) << "." <<
              (server->h_addr_list[0][1] & 0xff) << "." <<
              (server->h_addr_list[0][2] & 0xff) << "." <<
              (server->h_addr_list[0][3] & 0xff) << ", port " <<
              static_cast<int>(port) << std::endl;

    std::memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    std::memmove(&(serv_addr.sin_addr.s_addr), server->h_addr_list[0], 4);

    //connect to server
    if((connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))) < 0)
            error("ERROR on connecting");
    std::cout <<"Server connected, reading from server..."<<std::endl;

    char buffer[256];
    do
    {
        //clear buffer and get message from server
        std::memset(&buffer,0,sizeof(buffer));

        if(recv(sockfd,&buffer,sizeof(buffer),0) < 0)
            error("ERROR receiving message from server");

        std::cout <<"Server message: "<<buffer<<std::endl;

        //clear buffer and send message to server
        std::memset(&buffer,0,sizeof(buffer));

        std::cout <<"Enter message to send to server: "<<std::endl;
        std::cin.getline(buffer,sizeof(buffer));

        if(send(sockfd,&buffer,sizeof(buffer),0) < 0)
            error("ERROR sending message to server");

    }while(std::strcmp(buffer,"quit") != 0);

    close(sockfd);
}