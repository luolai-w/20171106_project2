#include "SGDBImplThread.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

void* SGDBImplThread::ThreadProc(void* pUserData)
{
    SGDBImplThread* p = reinterpret_cast<SGDBImplThread*>(pUserData);
    
    if (p == NULL)
    {
        //error
        return NULL;
    }

    p->Run();

    return NULL;
}

SGDBImplThread::SGDBImplThread():m_iStatus(THREADSTATUS_INIT)
{
}

SGDBImplThread::~SGDBImplThread()
{
}

int SGDBImplThread::Initialize()
{
    return 0;
}

int SGDBImplThread::Start()
{
    if (GetStatus() == THREADSTATUS_RUNNING)
    {
        return -1;
    }

    pthread_attr_init(&m_stThreadAttr);
    //pthread_attr_setdetachstate(&m_stThreadAttr,PTHREAD_CREATE_JOINABLE);//default
    //pthread_attr_setcope(&m_stThreadAttr,PTHREAD_SCOPE_SYSTEM);//default
    
    m_iStatus = THREADSTATUS_RUNNING;

    pthread_create(&m_pThread,&m_stThreadAttr,SGDBImplThread::ThreadProc,(void*)this);  
    
    fprintf(stdout,"start thread %u\n",(unsigned int)m_pThread);
    return 0;
}

int SGDBImplThread::Join()
{
    pthread_join(m_pThread,NULL);
    return 0;
}

int SGDBImplThread::Run()
{
    while (GetStatus() == THREADSTATUS_RUNNING)
    {
        //todo
        //test
        struct timeval tm;
        gettimeofday(&tm,NULL); 
        fprintf(stdout,"%u thread(%u)> hello world!\n",(unsigned int)tm.tv_sec,(unsigned int)m_pThread);

        sleep(1);
    }

    return 0;
}

