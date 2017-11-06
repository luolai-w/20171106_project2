#ifndef __SGMAINCTL_H
#define __SGMAINCTL_H
#include "SGGameConnHandler.h"
#include "SGEpollEventLoop.h"
#include <vector>
#include "SGLocalConfig.h"

enum SvrType{
    SVRTYPE_NONE,
    SVRTYPE_CLIENT, 
    SVRTYPE_CLIENT2, 
    SVRTYPE_CLIENT3, 
    SVRTYPE_SERVER,
    SVRTYPE_SERVER2,
    SVRTYPE_SERVER3,
    SVRTYPE_MAX
};


typedef int(*RoutineCheckFunc)(void*);

struct SvrArguments
{
    struct HostInfo m_stListener;
    int m_iType;
    char m_acSvrName[255];
};

class SGMainCtl
{
    public:
        enum enMainStatus
        {
            STATUS_NONE,
            STATUS_RUNNING,
            STATUS_STOP,
        };

        struct RoutineCheckInfo{
            void* m_pUserData;
            RoutineCheckFunc m_pCheckFunc;
            unsigned int m_uiLastCheckTime;
            unsigned int m_uiCheckIntvl;
        };

        SGMainCtl(const struct SvrArguments stSvrArgs);
        ~SGMainCtl();
        int Initialize();
        int InitSignal();
        int LoadAllConfig();
        int StartServerNetwork();
        SGGameConnHandler* GetGameHandle(); 
        void Run();
        int AddRoutineCheck(RoutineCheckFunc pFunc, void* pUserData, unsigned int uiIntvl);
        int HandleRoutineCheck();
        int HandleSignalCheck();
        int MakePidFile();
        SGEpollEventLoop* GetEventLoop();
        unsigned int GetGameTime();
    private:
        enMainStatus m_iRunStatus;
        SGGameConnHandler* m_pGameHandler;
        struct SvrArguments m_stSvrArgs;
        std::vector<struct RoutineCheckInfo> m_vcRoutineCheck;
};
extern SGMainCtl* g_pMainCtl;
#endif
