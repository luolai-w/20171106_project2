#include "SGListener.h"
#include "SGLogTrace.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include "SGMainCtl.h"
#include "SGConnector.h"
#include "SGConnsManager.h"
#include "SGObjectPool.h"
#include "SGLogicObjFactory.h"

SGListener::SGListener(SGEpollEventLoop* pEventLoop):m_pEventLoop(pEventLoop),m_iSocketFD(-1)
{
}


SGListener::~SGListener()
{
    
    ERROR_LOG("destruct SGListener %d\n",GetObjectID());//test
    if(m_iSocketFD > 0)
    {
        close(m_iSocketFD);
    }
}

int SGListener::Initialize()
{
    return 0;
}

int SGListener::StartListen(const char* szIP, unsigned short unPort, int iBacklog)
{
    m_iSocketFD = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if (m_iSocketFD < 0)
    {
        ERROR_LOG("socket failed(%d):%d\n",m_iSocketFD,errno);
        return -1;
    } 

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(unPort);
    if (szIP != NULL && strlen(szIP) > 0)
    {
         addr.sin_addr.s_addr = inet_addr(szIP);
    }else{
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    addr.sin_addr.s_addr = htonl(INADDR_ANY);//test

    int ret = bind(m_iSocketFD,(const struct sockaddr*)&addr, sizeof(struct sockaddr));
    if (ret < 0)
    {
        ERROR_LOG("bind(%d) %s,%d,failed(%d):%d\n",m_iSocketFD,szIP,unPort,ret,errno);
        return -2;
    }
    
    ret = listen(m_iSocketFD, iBacklog);
    if (ret < 0)
    {
        ERROR_LOG("listen(%d) %d failed(%d):%d\n",m_iSocketFD,iBacklog,ret,errno);
        return -3;
    }

    ret = SetNonBlock(m_iSocketFD);
    if (ret < 0)
    {
        ERROR_LOG("SetNonBlock(%d) failed got:%d\n",m_iSocketFD,ret);
        return -4;
    }

    m_pEventLoop->AddEvent(m_iSocketFD,EVENT_READ,CONSTRUCT_OBJECT_MIXID(GetObjectTag(),GetObjectID()));

    ANY_LOG("start listen at: port(%u),socket(%d)\n",unPort,m_iSocketFD);
    return 0;
}

int SGListener::AcceptConnect()
{
    if (m_iSocketFD < 0)
    {
        return -1;
    }

    struct sockaddr_in addr;
    socklen_t slen = sizeof(struct sockaddr_in); 
    int iFD = accept(m_iSocketFD,(struct sockaddr*)&addr,&slen);
    if (iFD < 0) 
    {
        ERROR_LOG("accept(%d) failed(%d):%d\n", m_iSocketFD, iFD, errno);
        return -2;
    }
    
    SGServiceConnector* conn = new (g_pServiceConnObjMng->CreateObjectMem()) SGServiceConnector();
    if (conn == NULL)
    {
        ERROR_LOG("new SGServiceConnector failed!\n");
        return -3;
    }
    conn->SetUpAcceptConnect(iFD,addr.sin_addr.s_addr,addr.sin_port);

    ANY_LOG("Accept a new connect: ip(%s),port(%u),fd(%d)\n",inet_ntoa(addr.sin_addr),addr.sin_port,iFD);
    return 0;
}

int SGListener::GetObjectTag()
{
    return OBJECT_TAG_LISTENER;
}

int SGListener::Close()
{
   if (m_iSocketFD > 0)
    {
        m_pEventLoop->DelEvent(m_iSocketFD);
        close(m_iSocketFD);
        m_iSocketFD = -1;
        ANY_LOG("close a listen: fd(%d)\n",m_iSocketFD);
    }

   return 0; 
}

int SGListener::SetNonBlock(int iFD)
{
    int flag = fcntl(iFD,F_GETFL,0);
    if (flag < 0)
    {
        ERROR_LOG("fcntl(%d) get flag failed(%d):%d\n",iFD,flag,errno);
        return -1;
    }

    flag |= O_NONBLOCK;

    int ret = fcntl(iFD,F_SETFL,flag);
    if (ret < 0)
    {
        ERROR_LOG("fcntl(%d) set flag failed(%d):%d\n",iFD,flag,errno);
        return -2;
    }
    return 0;
}
