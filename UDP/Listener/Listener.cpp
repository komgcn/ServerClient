//
// Created by zhihui on 12/27/19.
//

#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <cstring>

int main(int argc, char *argv[]){

    struct addrinfo hints, *serv_info, *s_ptr;
    struct sockaddr_storage peer_addr;
    int sockfd, addr_s, name_s;
    char buffer[1024];

    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);

    if(argc != 2){
        std::cerr << "Usage: "<< argv[0] << " port" << std::endl;
        return -1;
    }

    //set up struct addrinfo
    std::memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    addr_s = getaddrinfo(NULL, argv[1], &hints, &serv_info);
    if(addr_s != 0){
        std::cerr << "Erro on getaddrinfo: "<< gai_strerror(addr_s)<<std::endl;
        return -1;
    }

    //create socket and bind to a port
    for (s_ptr = serv_info; s_ptr != nullptr; s_ptr->ai_next){
        sockfd = socket(s_ptr->ai_family,s_ptr->ai_socktype,s_ptr->ai_protocol);
        if(sockfd == -1)
            continue;
        if(bind(sockfd,serv_info->ai_addr,serv_info->ai_addrlen) == 0)
            break;
        close(sockfd);
    }

    if(s_ptr == nullptr){
        std::cerr << "ERROR on binding." <<std::endl;
        return -1;
    }

    freeaddrinfo(serv_info);

    //Wait for data to arrive
    std::cout << "Listening on port " << argv[1] <<" ..." << std::endl;

    while(true) {
        std::memset(&buffer,0,sizeof(buffer));
        ssize_t recv_byte = recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&peer_addr, &peer_addr_len);
        if(recv_byte == -1)
            continue;

        //Deduce the sender's info
        char host[NI_MAXHOST], port[NI_MAXSERV];

        name_s = getnameinfo((struct sockaddr*)&peer_addr,peer_addr_len,host, NI_MAXHOST,port,NI_MAXSERV, NI_NUMERICSERV);
        if(name_s == 0)
            std::cout << "Received " << recv_byte << " bytes from " <<
                      host <<":"<< port << ". Message: "<< buffer<<std::endl;
        else
            std::cerr << "Error on getnameinfo: " << gai_strerror(name_s) << ". Message: "<< buffer<<std::endl;

        //Respond to sender
        if(sendto(sockfd, "For the Horde!",256,0, (struct sockaddr*)&peer_addr,peer_addr_len) == -1)
            std::cerr << "ERROR sending response to peer" << std::endl;
    }
}