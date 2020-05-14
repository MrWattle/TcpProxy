//#include "proxy_server.h"

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
//#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <string.h>

int main() 
{
    std::cout << "TCP PROXY SERVER" << std::endl;

//##########################_Make_PROXY_############################
    int sock, listener;
    struct sockaddr_in addr;
    int sock_client/*, sock_server*/;
    struct sockaddr_in addr_client;
    char buf[1024];
    int bytes_read;
    
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    listen(listener, 5);

//#######################_Connect_to_server_##########################
    int sock_server;
    struct sockaddr_in adrr_server;

    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_server < 0)
    {
        perror("socket");
        exit(1);
    }

    adrr_server.sin_family = AF_INET;
    adrr_server.sin_port = htons(3425); // или любой другой порт...
    adrr_server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sock_server, (struct sockaddr *)&adrr_server, sizeof(adrr_server)) < 0)
    {
        perror("connect");
        exit(2);
    }   

//__________________________Start_proxy_______________________
    while(1)
    {
        sock_client = accept(listener, NULL, NULL);
        if(sock_client < 0)
        {
            perror("sock_client: accept");
            exit(3);
        }

        while(1)
        {   
            bzero(buf, 1024);         
            bytes_read = recv(sock_client, buf, 1024, 0);
            if(bytes_read <= 0) 
                break;
            std::cout<<"TCP PROXY: MSG RECIVED>> "<<buf;
            // if(isfdtype(sock_server, S_IFSOCK) == 1)
            send(sock_server, buf, bytes_read, 0);
            
            bzero(buf, 1024);
            bytes_read = recv(sock_server, buf, 1024, 0);
            if(bytes_read <= 0) 
                break;
            std::cout<<"TCP PROXY: SERVER ANS>> "<<buf;
            send(sock_client, buf, bytes_read, 0);
        }
    
        // close(sock_client);
        // close(sock_server);
    }
    
    close(sock_client);
    close(sock_server);
    close(listener);
    return 0;
}