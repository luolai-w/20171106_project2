#ifndef __SGCONNSMANAGER__
#define __SGCONNSMANAGER__

#include "SGSingleton.h"
#include "SGConnector.h"
#include <vector>
#include <map>
#include <list>

class SGConnsManager
{
    public:
        SGConnsManager();
        ~SGConnsManager();

        int Initialize();
        SGConnector* GetAConnObj();
        int AddNewConn(SGConnector* conn);
        int OnConnSocketFDChg(SGConnector* conn, int oldFD = -1);
        int RemoveConn(int fd);
        int DestroyConn(SGConnector* conn);
        SGConnector* FindConn(int fd);
        SGConnector* FindConnByIdx(int index);
        int GetConnsSize(){ return m_vcAllConns.size();}
    private:
        std::vector<SGConnector*> m_vcAllConns;
        std::map<int,int>   m_mpConnsIdxs;
        std::list<SGConnector*> m_listConnPool;
    
};


class SGServiceConnsManager{
    public:
        struct ServiceConnStatus{
            TObjMixID m_iObjMixID;
            unsigned int m_uiLastCheckTime;
        };

        SGServiceConnsManager();
        ~SGServiceConnsManager();
        
        int Initialize();
        int AddNewConn(TObjMixID connID);
        int RemoveConn(TObjMixID connID);
        int CheckConns();
        int OnHeartbeat(SGConnector* pConn);
        int OnHandle(SGConnector* pConn,const char*pcData, int iLen);
        int SendMsgToAll(int cmd,const char*pcData);
        int SendMessage(SGConnector* pConn,int cmd, const char* pcData);
        static int OnRoutineCheckConn(void* pUserData);
    private:
        std::vector<struct ServiceConnStatus> m_vcAllConns;
};






typedef SGSingleton<SGServiceConnsManager> SGServiceConnsManagerSgl;
#define g_pConnsManager SGServiceConnsManagerSgl::Instance()

#endif
