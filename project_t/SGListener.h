#ifndef __SGLISTENER_H
#define __SGLISTENER_H

#include "SGObjectPool.h"
#include "SGEpollEventLoop.h"

class SGListener:public SGObject
{
    public:
        SGListener(SGEpollEventLoop* pEventLoop);
        ~SGListener();
        
        int Initialize();
        int AcceptConnect();
        int StartListen(const char* szIP, unsigned short unPort, int iBacklog);
        int Close();
        int GetSocketFD(){return m_iSocketFD;}
        int GetObjectTag();
        int SetNonBlock(int iFD);
    private:
        SGEpollEventLoop* m_pEventLoop;
        int m_iSocketFD;
};
#endif
