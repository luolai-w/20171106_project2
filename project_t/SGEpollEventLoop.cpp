#include "SGEpollEventLoop.h"
#include "SGLogTrace.h"
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "SGConnsManager.h"
#include "SGMainCtl.h"
#include "SGListener.h"
#include "SGConnector.h"
#include "SGLogicObjFactory.h"
#include "SGObjectPool.h"

int SGEventHanderDelegate::OnRead(SGEpollEventLoop* pEventLoop, int iFD, TObjMixID iObjMixID)
{
    ANY_LOG("trigger OnRead(%d)\n",iFD);//test
    if (g_pObjectFactory->IsListener(iObjMixID))
    {
        SGListener* pListener = g_pObjectFactory->GetListener(iObjMixID);
        if (pListener == NULL)
        {
            ERROR_LOG("g_pObjectFactory->GetListener(%u) got NULL\n",iObjMixID);
            pEventLoop->DelEvent(iFD);
            return -1;
        }
        pListener->AcceptConnect();
        return 0;
    }
    

    SGConnector* pConn = g_pObjectFactory->GetConnector(iObjMixID); 

    if (pConn != NULL)
    {
        int iRet = pConn->OnRecv();
        if (iRet < 0)
        {
            ERROR_LOG("pConn->OnRecv() fd:%d failed got:%d,close it\n",iFD,iRet);
            pConn->Close();
            g_pObjectFactory->DestroyConnector(iObjMixID);
            return -2;
        }
        
        iRet = pConn->HandlePacket();
        if (iRet < 0)
        {
            ERROR_LOG("pConn->HandlePacket() fd:%d failed got:%d\n",iFD,iRet);
            pConn->Close();
            g_pObjectFactory->DestroyConnector(iObjMixID);
            return -3;
        }
    }else{

        pEventLoop->DelEvent(iFD);
        return -4;
    }
    return 0;
}

int SGEventHanderDelegate::OnWrite(SGEpollEventLoop* pEventLoop,int iFD,TObjMixID iObjMixID)
{
    
    ANY_LOG("trigger OnWrite(%d)\n",iFD);//test
    if (g_pObjectFactory->IsListener(iObjMixID))
    {
        
        //SGListener* pListener = g_pObjectFactory->GetListener(iObjMixID);
        //if (pListener == NULL)
        //{
        //    ERROR_LOG("g_pObjectFactory->GetListener(%u) got null\n",iObjMixID);
        //    pEventLoop->DelEvent(iFD);
        //    return -1;
        //}
       
        return 0; 
        
    }

    SGConnector* pConn = g_pObjectFactory->GetConnector(iObjMixID);
    if (pConn != NULL)
    {
        int iRet = pConn->OnSend();
        if (iRet < 0)
        {
            ERROR_LOG("pConn->OnSend() fd:%d failed got:%d\n",iFD,iRet);
            return -1;
        }
    }else{

        pEventLoop->DelEvent(iFD);
        return -2;
    }

    return 0;
}

int SGEventHanderDelegate::OnError(SGEpollEventLoop* pEventLoop,int iFD,TObjMixID iObjMixID)
{
    ANY_LOG("trigger OnError(%d) %d\n",iFD,errno);//test
    if (g_pObjectFactory->IsListener(iObjMixID))
    {
        return 0;
    }

    SGConnector* pConn = g_pObjectFactory->GetConnector(iObjMixID);
    if (pConn != NULL)
    {
       ERROR_LOG("OnError close fd:%d\n",iFD);
       pConn->Close(); 
       g_pObjectFactory->DestroyConnector(iObjMixID);
    }else{
        pEventLoop->DelEvent(iFD);
    }

    return 0;
}

SGEpollEventLoop::SGEpollEventLoop(int iMaxEventSize):m_iEpollFD(-1),
m_iEpollEventSize(iMaxEventSize),
m_pstEpollEvent(NULL)
{
}


SGEpollEventLoop::~SGEpollEventLoop()
{
   if(m_iEpollFD > 0)
    {
        close(m_iEpollFD);
    } 
}

int SGEpollEventLoop::Initialize()
{
    m_iEpollFD = epoll_create(m_iEpollEventSize);
    if (m_iEpollFD < 0)
    {
        ERROR_LOG("epoll_create(%d) failed got:%d\n",m_iEpollEventSize,m_iEpollFD);
        return -1;
    }
    
    m_pstEpollEvent = new epoll_event[m_iEpollEventSize];
    if( m_pstEpollEvent == NULL)
    {
        ERROR_LOG("new epoll_event[%d] failed!\n",m_iEpollEventSize);
        return -2;
    }

    return 0;
}


int SGEpollEventLoop::AddEvent(int iFD,int iEventType,TObjMixID iObjMixID)
{
    struct epoll_event event;

    event.events = EPOLLERR | EPOLLHUP;

    if (iEventType & EVENT_READ)
    {
        event.events |= EPOLLIN; 
    }
    if (iEventType & EVENT_WRITE)
    {   
        event.events |= EPOLLOUT;
    }

    event.data.u64 = (uint64_t)iFD << 32 | iObjMixID;

    int ret = epoll_ctl(m_iEpollFD,EPOLL_CTL_ADD,iFD,&event);
    if (ret < 0)
    {
        ERROR_LOG("epfd(%d) fd(%d) epoll_ctl failed(%d):%d\n", m_iEpollFD, iFD, ret,errno); 
        return ret;
    }
    ANY_LOG("AddEvent fd(%d) ok\n",iFD);//test
    return 0;
}

int SGEpollEventLoop::ChgEvent(int iFD,int iEventType,TObjMixID iObjMixID)
{
    struct epoll_event event;
    event.events = EPOLLERR | EPOLLHUP;
    
    if (iEventType & EVENT_READ)
    {
        event.events |= EPOLLIN;
    }
    
    if (iEventType & EVENT_WRITE)
    {
        event.events |= EPOLLOUT;
    }
    
    event.data.u64 = (uint64_t)iFD << 32 | iObjMixID;

    int ret = epoll_ctl(m_iEpollFD,EPOLL_CTL_MOD,iFD,&event);
    if (ret < 0)
    {
        ERROR_LOG("epfd(%d) fd(%d) epoll_ctl failed(%d):%d\n", m_iEpollFD, iFD, ret,errno); 
        return ret;
    }
    return 0;
}


int SGEpollEventLoop::DelEvent(int iFD)
{
   ERROR_LOG("DelEvent fd(%d)\n",iFD);
   struct epoll_event event;
   int ret = epoll_ctl(m_iEpollFD, EPOLL_CTL_DEL,iFD,&event);
   if (ret < 0)
   {
        ERROR_LOG("epfd(%d) fd(%d) epoll_ctl failed(%d):%d\n", m_iEpollFD, iFD, ret,errno); 
        return ret;
   } 
   return 0; 
}

int SGEpollEventLoop::LoopOnce(int iTimeoutMS)
{
    struct epoll_event* evt = NULL;

    int triggerNum = epoll_wait(m_iEpollFD, m_pstEpollEvent, m_iEpollEventSize,iTimeoutMS);    
    if (triggerNum < 0)
    {
        

        if (errno == EINTR)
        {
            ANY_LOG("epfd(%d) epoll_wait %d %d failed(%d):%d\n",m_iEpollFD, m_iEpollEventSize,iTimeoutMS,triggerNum,errno);       
            return 0;//try again
        }

        ERROR_LOG("epfd(%d) epoll_wait %d %d failed(%d):%d\n",m_iEpollFD, m_iEpollEventSize,iTimeoutMS,triggerNum,errno);       

        return -1;
    }

    for (int i = 0; i < triggerNum; i++)
    {
        evt = m_pstEpollEvent + i;     
        int iFD = (int)(evt->data.u64 >> 32);
        TObjMixID iObjMixID = (TObjMixID)evt->data.u64;
        if ((evt->events & EPOLLERR) || (evt->events & EPOLLHUP))
        {
            g_SGEventHander->OnError(this,iFD,iObjMixID);
        }else{

            if (evt->events & EPOLLIN)
            {
                g_SGEventHander->OnRead(this,iFD,iObjMixID);
            }

            if (evt->events & EPOLLOUT)
            {
                g_SGEventHander->OnWrite(this,iFD,iObjMixID);
            } 
        }         
    }

    return 0;
}

