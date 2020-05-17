#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <iostream>
#include <string.h>

#include "socket_wrap.h"

using std::cerr;
using std::cout;
using std::endl;

int main() 
{
    int listener, sock_server, sock_client;
    struct sockaddr_in addr, addr_server;
    char buf[1024];
    int bytes_read;

//##########################_Make_PROXY_############################
    listener = socket(AF_INET, SOCK_STREAM, 0);
    SocketWrap sw1(listener);
    if(listener < 0)
    {
        cerr<<"Error: listener socket create"<<endl;
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cerr<<"Error: listener bind"<<endl;
        return 1;
    }

    listen(listener, 5);

//#######################_Connect_to_server_##########################
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    SocketWrap sw2(sock_server);
    if(sock_server < 0)
    {
        cerr<<"Error: server socket create"<<endl;
        return 1;
    }

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(3425);
    addr_server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sock_server, (struct sockaddr *)&addr_server, sizeof(addr_server)) < 0)
    {
        cerr<<"Error: connecting to server"<<endl;
        return 1;
    }   

//###########################_Start_proxy_############################
    while(true)
    {
        sock_client = accept(listener, NULL, NULL);
        SocketWrap sw3(sock_client);
        if(sock_client < 0)
        {
            cerr<<"Error: accepting from client socket"<<endl;
            return 1;
        }

        while(true)
        {   
            bzero(buf, 1024);         
            bytes_read = recv(sock_client, buf, 1024, 0);
            if(bytes_read <= 0) 
                break;
            cout<<"TCP PROXY: MSG RECIVED>> "<<buf;
            send(sock_server, buf, bytes_read, 0);
            
            bzero(buf, 1024);
            bytes_read = recv(sock_server, buf, 1024, 0);
            if(bytes_read <= 0) 
                break;
            cout<<"TCP PROXY: SERVER ANS>> "<<buf;
            send(sock_client, buf, bytes_read, 0);
        }
    }
    
    return 0;
}