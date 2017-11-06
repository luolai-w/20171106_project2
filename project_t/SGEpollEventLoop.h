#ifndef __SGEPOLLEVENTLOOP_H
#define __SGEPOLLEVENTLOOP_H
#include "SGSingleton.h"
#include <sys/epoll.h>
#include "SGObjectPool.h"

enum EVENT_TYPE{
    EVENT_READ = 1 << 1,
    EVENT_WRITE = 1 << 2
};


class SGEpollEventLoop;

class SGEventHanderDelegate{
    public:
        SGEventHanderDelegate(){}
        ~SGEventHanderDelegate(){}
        
        int OnRead(SGEpollEventLoop* pEventLoop,int iFD,TObjMixID iObjMixID);
        int OnWrite(SGEpollEventLoop* pEventLoop, int iFD, TObjMixID iObjMixID);
        int OnError(SGEpollEventLoop* pEventLoop, int iFD,TObjMixID iObjMixID);
};

typedef SGSingleton<SGEventHanderDelegate> SGEventHanderDelegateSgl;
#define g_SGEventHander SGEventHanderDelegateSgl::Instance()

class SGEpollEventLoop{
    public:
    SGEpollEventLoop(int iMaxEventSize);
    ~SGEpollEventLoop();
    int Initialize();
    
    int AddEvent(int iFD,int iEventType,TObjMixID objMixID);
    int ChgEvent(int iFD,int iEventType,TObjMixID objMixID);
    int DelEvent(int iFD);
    int LoopOnce(int iTimeoutMS);

    private:

    int m_iEpollFD;
    int m_iEpollEventSize;
    epoll_event* m_pstEpollEvent;
};

#endif
