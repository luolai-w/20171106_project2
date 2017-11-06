#include "SGMainCtl.h"
#include "SGLogTrace.h"
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string>
#include <iostream>
#include "SGConfigReader.h"
#include "SGLogicObjFactory.h"
#include "SGObjectPool.h"

enum{
    SIGNAL_NONE,
    SIGNAL_QUIT,
    SIGNAL_RELOAD,
    SIGNAL_MSG_IN
};

#define SIG_USER1 (SIGRTMIN + 1)
#define SIG_USER2 (SIGRTMIN + 2)
#define SIG_USER3 (SIGRTMIN + 3)

volatile static int g_iSignalFlag = SIGNAL_NONE;

void SignalUser1Handler(int signal)
{
    //ANY_LOG("got a signal: SIG_USER1(%d)!\n",signal);//test
    //todo
    
    g_iSignalFlag = SIGNAL_QUIT;
    //signal(SIG_USER1,SignalUser1Handler);
}

void SignalUser2Handler(int signal)
{
    ANY_LOG("got a signal: SIG_USER2(%d)!\n",signal);//test
    //todo

    g_iSignalFlag = SIGNAL_RELOAD;
    //signal(SIG_USER2,SignalUser2Handler);
}

void SignalUser3Handler(int signal)
{
    ANY_LOG("got a signal: SIG_USER3(%d)!\n",signal);//test
    //todo
    
    g_iSignalFlag = SIGNAL_MSG_IN;
    //signal(SIG_USER3,SignalUser3Handler);
}


SGMainCtl::SGMainCtl(const struct SvrArguments stSvrArgs):m_iRunStatus(STATUS_NONE),
m_pGameHandler(NULL),
m_stSvrArgs(stSvrArgs)
{
}


SGMainCtl::~SGMainCtl()
{

    if (m_pGameHandler != NULL)
    {
        delete m_pGameHandler;
        m_pGameHandler = NULL;
    }
}

int SGMainCtl::MakePidFile()
{
    char szPidFileName[255] = {0};
    
    snprintf(szPidFileName,sizeof(szPidFileName),"%s.pid",m_stSvrArgs.m_acSvrName); 
    int iFD = open(szPidFileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR|S_IRGRP|S_IROTH);
    if (iFD < 0)
    {
        ERROR_LOG("open pid file %s failed got:%d\n",szPidFileName,iFD);
        return -1;
    }
    
    int iRet = ftruncate(iFD,0);
    if (iRet < 0)
    {
        ERROR_LOG("ftruncate %s file failed(%d)\n",szPidFileName,errno);
        return -2;
    }

    char szPidBuff[16] = {0};
    int iLen = snprintf(szPidBuff,sizeof(szPidBuff),"%d\n",static_cast<int>(getpid()));
    
    iRet = write(iFD, szPidBuff,iLen);
    if (iRet < 0)
    {
        ERROR_LOG("write file %s failed(%d)\n",szPidBuff,errno);
        return -3;
    }
    
    close(iFD);

    return 0;
}

int SGMainCtl::Initialize()
{
    MakePidFile();

    int iRet = LoadAllConfig();
    if (iRet)
    {
        ERROR_LOG("LocalAllConfig failed(%d)\n",iRet);
        return -1;
    }

    
    iRet = g_pObjectFactory->Initialize();
    if (iRet)
    {
        ERROR_LOG("g_pObjectFactory->Initialize() failed:%d\n",iRet);
        return -3;
    }

   // m_pEventLoop = new SGEpollEventLoop(100*10000);
   // if (!m_pEventLoop)
   // {
   //     ERROR_LOG("new SGEpollEventLoop failed!\n");
   //     return -2;
   // }
   // 
   // iRet = m_pEventLoop->Initialize();
   // if (iRet)
   // {
   //     ERROR_LOG("m_pEventLoop->Initialize() failed got:%d\n",iRet);
   //     return -3;
   // }
   // 
   // m_pListener = new SGListener();
   // if (!m_pListener)
   // {
   //     ERROR_LOG("new SGListener() failed!\n");
   //     return -4;
   // }

    iRet = StartServerNetwork();
    if (iRet)
    {
        ERROR_LOG("StartServerNetwork() failed got:%d\n",iRet);
        return -5;
    }

    m_pGameHandler = GetGameHandle();
    if (!m_pGameHandler)
    {
        //return -3;
    }

    if (m_pGameHandler != NULL)
    {
    
        int ret = m_pGameHandler->Initialize();
        if (ret)
        {
            ERROR_LOG("m_pGameHandler->Initialize() failed got:%d\n",ret);
            return -6;
        }
    }

    iRet = InitSignal();

    return 0;
}

int SGMainCtl::InitSignal()
{
    signal(SIG_USER1,SignalUser1Handler);
    signal(SIG_USER2,SignalUser2Handler);
    signal(SIG_USER3,SignalUser3Handler);

    return 0;
}

int SGMainCtl::LoadAllConfig()
{
    
    int iRet = g_pLocalConfig->Initialize("../conf/local.conf");
    if (iRet)
    {
        ERROR_LOG("initialize local config failed(%d)\n",iRet);
        return -1;
    }
    
    iRet = g_pLocalConfig->LoadConfig(false);
    if (iRet)
    {
        ERROR_LOG("load config failed(%d)\n",iRet); 
        return -2;
    }

    g_pLocalConfig->TraceConfig();

    return 0;
}

int SGMainCtl::StartServerNetwork()
{
    int iMixID = CONSTRUCT_OBJECT_MIXID(OBJECT_TAG_LISTENER,0);
    SGListener* pListener = g_pObjectFactory->GetListener(iMixID);
    if (pListener == NULL)
    {
        ERROR_LOG("g_pObjectFactory->GetListener(%d) failed\n",iMixID);
        return -1;
    }

    int iRet = pListener->Initialize();
    if (iRet)
    {
        ERROR_LOG("pListener->Initialize() failed got:%d\n",iRet);
        return -2;
    }
    
    if (g_pLocalConfig->m_iListenPort == 0)
    {
        return 0;
    }

    iRet = pListener->StartListen(g_pLocalConfig->m_acListenIP,g_pLocalConfig->m_iListenPort,10000); 
    if (iRet)
    {
        ERROR_LOG("pListener->StartListen(%s,%u,10000) failed got:%d\n",g_pLocalConfig->m_acListenIP,g_pLocalConfig->m_iListenPort);
        return -3;
    }

    return 0;
}

SGGameConnHandler* SGMainCtl::GetGameHandle()
{
    if (m_pGameHandler != NULL)
    {
        return m_pGameHandler;
    }

    switch(g_pLocalConfig->m_iSvrType)
    {
        case SVRTYPE_CLIENT:
            m_pGameHandler = new SGGameConnHandler();
        break;
        case SVRTYPE_SERVER:
        break;
        default:
            m_pGameHandler = new SGGameConnHandler();
        break;
    }
    
    return m_pGameHandler;
}

SGEpollEventLoop* SGMainCtl::GetEventLoop()
{
    return g_pObjectFactory->GetEventLoop();
}

int SGMainCtl::AddRoutineCheck(RoutineCheckFunc pFunc, void* pUserData, unsigned int uiIntvl)
{
    struct RoutineCheckInfo stInfo;
    
    std::vector<struct RoutineCheckInfo>::iterator itr = m_vcRoutineCheck.begin();
    
    for (;itr != m_vcRoutineCheck.end(); itr++)
    {
        if(itr->m_pCheckFunc == pFunc && itr->m_pUserData == pUserData)
        {
            ERROR_LOG("duplicate routine check handler");
            return -1;
        }
    }

    stInfo.m_pUserData = pUserData;
    stInfo.m_pCheckFunc = pFunc;
    stInfo.m_uiCheckIntvl = uiIntvl;

    struct timeval tvTime;
    gettimeofday(&tvTime,NULL); 
    stInfo.m_uiLastCheckTime = tvTime.tv_sec; 

    m_vcRoutineCheck.push_back(stInfo);
    
    return 0;
}

unsigned int SGMainCtl::GetGameTime()
{
    struct timeval tvTime;
    
    gettimeofday(&tvTime,NULL);
    
    return tvTime.tv_sec;
}

int SGMainCtl::HandleRoutineCheck()
{
    struct timeval tvTime;
    gettimeofday(&tvTime,NULL);
    unsigned int uiNowTime = tvTime.tv_sec;

    std::vector<struct RoutineCheckInfo>::iterator itr = m_vcRoutineCheck.begin();
    for(;itr != m_vcRoutineCheck.end();itr++)
    {
       if (uiNowTime >= itr->m_uiLastCheckTime + itr->m_uiCheckIntvl)
       {
            itr->m_uiLastCheckTime = uiNowTime;
            
            itr->m_pCheckFunc(itr->m_pUserData);
       } 
    }    

    return 0;
}

int SGMainCtl::HandleSignalCheck()
{
    if (g_iSignalFlag == SIGNAL_NONE)
    {
        return 0;
    }

    switch(g_iSignalFlag)
    {
        case SIGNAL_QUIT:

            m_iRunStatus = SGMainCtl::STATUS_STOP;

            break;
        case SIGNAL_RELOAD:
            {
                ANY_LOG("handle signal->SIGNAL_RELOAD\n"); 
                SGConfigReader reader;
                reader.ReadFile(g_pLocalConfig->m_acFileName);
            }

            break;
        case SIGNAL_MSG_IN:
            {
                SGGameConnHandler* pGameHandler = GetGameHandle();
                if (pGameHandler != NULL)
                {
                    std::string str;
                    std::cout<<"enter message:"<<std::endl;
                    std::cin >> str;
                    ANY_LOG("SIGNAL_MSG_IN: %s\n",str.c_str());
                    if (!str.empty())
                    {
                        pGameHandler->SendMsgToAllServers(2,str.c_str());
                    }
                }            
            }

            break;
        default:

            ANY_LOG("handle signal(%d)->do nothing\n",g_iSignalFlag);
            break;
         
    }

    g_iSignalFlag = SIGNAL_NONE;

    return 0;
}

void SGMainCtl::Run()
{
    ANY_LOG("----------------------Run Begin()----------------\n");
    m_iRunStatus = SGMainCtl::STATUS_RUNNING;
     
    while(m_iRunStatus == STATUS_RUNNING)
    {

        HandleSignalCheck();
        HandleRoutineCheck();

        SGEpollEventLoop* pEventLoop = g_pObjectFactory->GetEventLoop();
        if (pEventLoop != NULL)
        {
            pEventLoop->LoopOnce(1000);
        }                

    }

    ANY_LOG("----------------------Run End()----------------\n");
}
