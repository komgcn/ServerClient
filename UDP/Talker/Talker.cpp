//
// Created by zhihui on 12/27/19.
//

#include <iostream>
#include <netdb.h>
#include <cstring>
#include <unistd.h>

int main(int argc, char *argv[]){

    struct addrinfo hint, *result, *r_ptr;
    int sockfd, addr_code;

    if(argc != 3){
        std::cerr << "Usage: " << argv[0] << " hostname port" << std::endl;
    }

    //Initiate struct addrinfo used for creating socket
    std::memset(&hint, 0 ,sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_PASSIVE;
    hint.ai_protocol = 0;
    hint.ai_addr = NULL;
    hint.ai_canonname = NULL;
    hint.ai_next = NULL;

    addr_code = getaddrinfo(argv[1], argv[2], &hint, &result);
    if(addr_code != 0){
        std::cerr << "Error getaddrinfo: "<< gai_strerror(addr_code)<<std::endl;
        return -1;
    }

    //create socket
    for(r_ptr = result; r_ptr != nullptr; r_ptr->ai_next) {
        sockfd = socket(r_ptr->ai_family,r_ptr->ai_socktype,r_ptr->ai_protocol);
        if(sockfd == -1)
            continue;
        break;
    }

    if(r_ptr == nullptr){
        std::cerr << "ERROR creating socket" << std::endl;
        return -1;
    }

    //send messages from input prompt to socket
    char buffer[1024];
    std::cout << "Ready to fire..." << std::endl;
    while(true) {
        std::cin.getline(buffer,sizeof(buffer));
        if(sendto(sockfd,buffer,sizeof(buffer),0,r_ptr->ai_addr,r_ptr->ai_addrlen) == -1){
            std::cerr << "ERROR on sending" <<std::endl;
            return -1;
        }
    }
}
