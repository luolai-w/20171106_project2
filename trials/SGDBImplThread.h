#ifndef __SGDBIMPLTHREAD_H
#define __SGDBIMPLTHREAD_H
#include <pthread.h>

enum ThreadStatus{
    THREADSTATUS_INIT,
    THREADSTATUS_RUNNING,
    THREADSTATUS_STOPPED, 
};

class SGDBImplThread{
    public:
        SGDBImplThread();
        ~SGDBImplThread();
        int Initialize();
        int Start();
        int Join();
        int Stop(){
            if (m_iStatus == THREADSTATUS_RUNNING)
            {
                m_iStatus = THREADSTATUS_STOPPED;
                return 0;
            }

            return -1;
        } 
        
        int GetStatus() {return m_iStatus;}

    protected:
        int Run();
        static void* ThreadProc(void* pUserData);

    protected:
        volatile int m_iStatus;
        pthread_t m_pThread; 
        pthread_attr_t m_stThreadAttr;
};

#endif
