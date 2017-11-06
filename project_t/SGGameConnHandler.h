#ifndef __SGGAMECONNHANDLER_H
#define __SGGAMECONNHANDLER_H
#include <vector>
#include "SGConnector.h"

#define MSG_MAX_LENGTH (2*1024)

struct ConnStatus{
    int m_iSvrID;
    int m_iObjMixID;
    unsigned int m_uiLastHeartbeatTime;
};

class SGGameConnHandler{
    public:
        SGGameConnHandler();
        ~SGGameConnHandler();
        
        int Initialize();
        int InitConns();
        int CheckConns();
        int CheckHeartbeat();
        int OnHandle(SGConnector* pConn,const char* pcPacketData,int iPacketLength);
        int SendMsgToAllServers(int cmd,const char* pcData);
        int SendMessage(SGConnector* pConn, int cmd,const char* pcData);
        static int OnRoutineCheckConn(void* pUserData);
    private:
    std::vector<struct ConnStatus> m_allConns;
};


#endif
