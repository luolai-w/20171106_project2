#include "SGGameConnHandler.h"
#include <sys/time.h>
#include "SGMainCtl.h"
#include "SGLogTrace.h"
#include <cstring>
#include "SGLogicObjFactory.h"
#include "SGGameConnector.h"

SGGameConnHandler::SGGameConnHandler()
{
    
}


SGGameConnHandler::~SGGameConnHandler()
{
}

int SGGameConnHandler::Initialize()
{
    InitConns();

    int iRet = g_pMainCtl->AddRoutineCheck(OnRoutineCheckConn,this,g_pLocalConfig->m_iGameConnCheckIntvl);
    if (iRet)
    {
        ERROR_LOG("g_pMainCtl->AddRoutineCheck failed got:%d\n",iRet);
    
        return -1;
    }

    return 0;
}
    
int SGGameConnHandler::InitConns()
{

    for(int i = 0; i < (int)g_pLocalConfig->m_vcGameHosts.size(); i++)
    {

        struct ConnStatus stConnStatus;
        SGGameConnector* conn = new (g_pGameConnObjMng->CreateObjectMem())SGGameConnector();
        if (conn == NULL)
        {
            ERROR_LOG("new SGGameConnector() failed\n");
            stConnStatus.m_iObjMixID = -1;
            m_allConns.push_back(stConnStatus);
            continue;
        }

        stConnStatus.m_iSvrID = -1;
        stConnStatus.m_iObjMixID = CONSTRUCT_OBJECT_MIXID(conn->GetObjectTag(),conn->GetObjectID());
        stConnStatus.m_uiLastHeartbeatTime = 0;

        m_allConns.push_back(stConnStatus);
    }

    return 0;
}


int SGGameConnHandler::CheckConns()
{
    struct ConnStatus* pStatus = NULL;
    for(int i = 0; i <(int)m_allConns.size(); i++)
    {
        pStatus = &m_allConns[i];

        SGConnector* pConn = g_pObjectFactory->GetConnector(pStatus->m_iObjMixID);
        if (pConn == NULL)
        {
            ERROR_LOG("g_pObjectFactory->GetConnector(%d) failed\n",pStatus->m_iObjMixID);
            continue;
        }
    
        if (pConn->IsConnected())
        {
            continue;
        }

        struct HostInfo* pHostInfo = &g_pLocalConfig->m_vcGameHosts[i];
        
        ANY_LOG("try to connect(%s:%u)\n",pHostInfo->m_acIP,pHostInfo->m_usPort);

        int iRet = pConn->ConnectTo(pHostInfo->m_acIP,pHostInfo->m_usPort);
        if (iRet < 0)
        {
            ERROR_LOG("pConn->ConnectTo(%s:%u) failed,got:%d\n",pHostInfo->m_acIP,pHostInfo->m_usPort,iRet);
            continue;
        }

        ANY_LOG("connected!\n");

    }

    return 0;
}

int SGGameConnHandler::OnRoutineCheckConn(void* pUserData)
{
    SGGameConnHandler* pThis = reinterpret_cast<SGGameConnHandler*>(pUserData);
    if (pThis == NULL)
    {
        return -1;
    }
    
    pThis->CheckConns();

    pThis->CheckHeartbeat();

    return 0;
}


int SGGameConnHandler::CheckHeartbeat()
{
    
    struct timeval tvTime;

    gettimeofday(&tvTime,NULL);

    unsigned int uiNowTime = tvTime.tv_sec;


    char acTmpMsg[255] = {0};
    SGLogTrace::GetStringTime(tvTime,acTmpMsg,sizeof(acTmpMsg));
    int iLen = strlen(acTmpMsg);
    snprintf(acTmpMsg + iLen, sizeof(acTmpMsg)- iLen,":%s","hello world!");

    struct ConnStatus* pStatus = NULL;
    for(int i = 0; i < (int)m_allConns.size(); i++)
    {
       pStatus = &m_allConns[i]; 
       SGConnector* pConn = g_pObjectFactory->GetConnector(pStatus->m_iObjMixID);
       if (pConn == NULL || !pConn->IsConnected())
       {
           continue;
       }
           
       if (uiNowTime - pStatus->m_uiLastHeartbeatTime < 30)
       {
           continue;
       } 

       SendMessage(pConn,1,acTmpMsg);
    }

    return 0;
}


int SGGameConnHandler::OnHandle(SGConnector* pConn,const char* pcPacketData,int iPacketLength)
{
    return 0;
}

int SGGameConnHandler::SendMsgToAllServers(int cmd,const char* pcData)
{
    
    struct ConnStatus* pStatus = NULL;
    for(int i = 0; i < (int)m_allConns.size(); i++)
    {
        pStatus = &m_allConns[i];        
        SGConnector* pConn = g_pObjectFactory->GetConnector(pStatus->m_iObjMixID);
        if (pConn != NULL && pConn->IsConnected())
        {
            SendMessage(pConn,cmd,pcData);
        } 

    } 
    return 0;
}

int SGGameConnHandler::SendMessage(SGConnector* pConn, int cmd,const char* pcData)
{
    if (pConn == NULL)
    {
        return -1;
    }
    
    char szTempBuffer[MSG_MAX_LENGTH]; 
    int iPacketHeadLen = sizeof(int);
    int iCommandLen = sizeof(int);

    int iOffsetPos = 0 +iPacketHeadLen + iCommandLen; 

    int iLen = strlen(pcData);

    if (MSG_MAX_LENGTH < iOffsetPos)
    {
        ERROR_LOG("error allocation is not enough. need(%d),real(%d)\n",iOffsetPos,MSG_MAX_LENGTH);
        return -2;
    }

    if (iLen > MSG_MAX_LENGTH - iOffsetPos)
    {
        ERROR_LOG("msg is too long(%d)\n",iLen);
        iLen = MSG_MAX_LENGTH - iOffsetPos;
    }

    memcpy(szTempBuffer + iOffsetPos,pcData,iLen);
    
    *((int*)(szTempBuffer + iOffsetPos - iCommandLen)) = cmd;

    iOffsetPos -= iCommandLen;
    iLen += iCommandLen;

    *((int*)(szTempBuffer + iOffsetPos - iPacketHeadLen)) = iLen;
    
    iOffsetPos -= iPacketHeadLen;
    iLen += iPacketHeadLen;

    int ret = pConn->Send(szTempBuffer, iLen);
    if (ret)
    {
        ERROR_LOG("pConn->Send failed length(%d) got:%d\n",iLen,ret);
        return -3;
    }
    ANY_LOG("SendMessage ok! length(%d),cmd(%d),msg(%s)\n",iLen,cmd,pcData);//test
    ANY_LOG("pConn->Send(%s,%d)\n",szTempBuffer,iLen);

    return 0; 
}
