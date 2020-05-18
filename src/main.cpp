#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
// #include <stdio.h> 
#include <stdlib.h>
#include <sys/stat.h>

#include <iostream>
#include <string.h>

#include "socket_wrap.h"

#include <thread>
#include <mutex>

using std::cerr;
using std::cout;
using std::endl;

std::mutex mutex_sock_server;

void client_processing(int sock_client, int sock_server)
{
    cout<<"thread id = "<<std::this_thread::get_id()<<endl;
    SocketWrap swc(sock_client);
    char buf[1024];
    int bytes_read;

    bzero(buf, 1024);         
    bytes_read = recv(sock_client, buf, 1024, 0);
    if(bytes_read <= 0) 
        return;
    cout<<"TCP PROXY: MSG RECIVED>> "<<buf;
    
    // std::lock_guard<std::mutex> m_guard(mutex_sock_server);
    mutex_sock_server.lock();
    {
        send(sock_server, buf, bytes_read, 0);
        bzero(buf, 1024);
        bytes_read = recv(sock_server, buf, 1024, 0);
    }
    mutex_sock_server.unlock();
    
    if(bytes_read <= 0) 
        return;
    cout<<"TCP PROXY: SERVER ANS>> "<<buf;

    send(sock_client, buf, bytes_read, 0);
}

int main() 
{
    int listener, sock_server, sock_client;
    struct sockaddr_in addr, addr_server;

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
        if(sock_client < 0)
        {
            cerr<<"Error: accepting from client socket"<<endl;
            continue;
        }

        std::thread thrd_client(client_processing, sock_client, sock_server);
        thrd_client.detach();
    }
    
    return 0;
}