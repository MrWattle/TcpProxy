#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <iostream>
#include <string.h>

#include "socket_wrap.h"
#include "tls_parser.cpp"

#include <thread>
#include <fcntl.h>
#include <time.h>

#define BUF_SIZE 32768

using std::cerr;
using std::cout;
using std::endl;

void client_processing(int sock_client, int sock_server)
{
    cout<<"thread id = "<<std::this_thread::get_id()<<endl;
    SocketWrap swc(sock_client);
    char buf[BUF_SIZE];
    ssize_t bytes_read, bytes_write;
    const timespec sleep_interval{.tv_sec = 0, .tv_nsec = 10000};

    const int cl_flags = fcntl(sock_client, F_GETFL, 0);
    fcntl(sock_client, F_SETFL, cl_flags | O_NONBLOCK);

    const int sr_flags = fcntl(sock_server, F_GETFL, 0);
    fcntl(sock_server, F_SETFL, sr_flags | O_NONBLOCK);

    while (true)
    {
        // Read from client -> server
        bytes_read = read(sock_client, buf, BUF_SIZE);
        if (bytes_read < 0)
        {
            if (errno != EWOULDBLOCK) {
                std::cerr << "Error: read(sock_client)" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            bytes_write = write(sock_server, buf, bytes_read);
            std::cout << "client -> server : " << bytes_read << '\\' << bytes_write << std::endl;
        }
        // Read from server -> client
        bytes_read = read(sock_server, buf, BUF_SIZE);
        if (bytes_read < 0)
        {
            if (errno != EWOULDBLOCK) {
                std::cerr << "Error: read(sock_server)" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            bytes_write = write(sock_client, buf, bytes_read);
            std::cout << "server -> client : " << bytes_read << '\\' << bytes_write << std::endl;
        }

        nanosleep(&sleep_interval, nullptr);
    }
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
    addr_server.sin_port = htons(443);
    addr_server.sin_addr.s_addr = htonl((uint32_t)0x34323DBF);//52.50.61.191 (en.privatbank.ua)
    if(connect(sock_server, (struct sockaddr *)&addr_server, sizeof(addr_server)) < 0)
    {
        cerr<<"Error: connecting to server"<<endl;
        return 1;
    }

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
