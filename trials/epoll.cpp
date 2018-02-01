#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_EPOLL_NUM 1000
#define MAX_BUFF_SIZE 1024

int g_iEpollFD = -1;

epoll_event* g_pstEpollEvent = NULL;

int g_listenerSocket = -1;
std::vector<int> g_clientSockets;

char g_buff[MAX_BUFF_SIZ] = {0};

int setnonblock(int fd)
{
    int flag;
    if(flag = fcntl(fd, F_GETFL,0) < 0)
    {
        perror("get flag");
        return -1;
    }

    flag |= O_NONBLOCK;

    if(fcntl(fd, F_SETFL, flag) < 0)
    {
        perror("set flag");
        return -1;
    }
    
    return 0;
}

int start()
{
    g_iEpollFD = epoll_create(MAX_EPOLL_NUM);
    if (m_iEpollFD < 0) 
    {
        return -1
    }

    m_pstEpollEvent = new epoll_event[MAX_EPOLL_NUM];
    if (m_pstEpollEvent)
    {
        return -1
    }

    return 0;
}

int update()
{

}

int end()
{
}

void run()
{
    while(1)
    {
        int iEpollNum = epoll_wait(g_iEpollFD,m_pstEpollEvent,MAX_EPOLL_NUM, 100);
        for (int i = 0; i < iEpollNum; i++)
        {
            int socket_fd = m_pstEpollEvent[i].data.fd;
            unsigned int uiEvent = m_pstEpollEvent[i].events;
            if (uiEvent & EPOLLIN)
            {
                if (socket_fd == g_listenerSocket)
                {
                    sockaddr_in stConnAddr;
                    socklen_t iAddrLength = sizeof(stConnAddr);
                    int new_socket = accept(listenerSocket,(struct sockaddr*) &stConnAddr,&iAddrLength);
                    if(new_socket < 0)
                    {
                        perror("accept failed");
                        continue;
                    }
                    
                    setnonblock(new_socket);
                    struct epoll_event evt;
                    evt.events = EPOLLIN | EPOLLET;
                    evt.data.fd = new_socket;
                    if(epoll_ctl(g_iEpollFD,EPOLL_CTL_ADD,new_socket,&evt))
                    {
                        perror("epoll_ctl failed");
                        close(new_socket);
                        continue;
                    }
                    g_clientSockets.push_back(new_socket); 
                     
                }
                else if(std::find(g_clientSockets.begin(),g_clientSockets.end(),socket_fd) != g_clientSockets.end())
                {
                    int nRead = read(socket_fd,g_buff,MAX_BUFF_SIZE);
                }
            }
        }
    }    
}

int main(int argn, char* argv[])
{
    int handle =  epoll_create(10);

    start();
    run();
    end()

    return 0;
}
