#include "SGConnsManager.h"
#include "SGLogTrace.h"
#include "SGMainCtl.h"
#include "SGLogicObjFactory.h"

#define MAX_SEND_MSG_LENGTH 1024

SGConnsManager::SGConnsManager()
{
}


SGConnsManager::~SGConnsManager()
{
    for ( int i = 0; i< (int)m_vcAllConns.size(); i++)
    {
        SGConnector* pConn = m_vcAllConns[i];
        if (pConn != NULL)
        {
            pConn->Destroy();
            delete pConn;
        }
    }
    m_vcAllConns.clear();
    m_mpConnsIdxs.clear();

    for (std::list<SGConnector*>::iterator itr = m_listConnPool.begin(); itr != m_listConnPool.end();itr++)
    {
        delete *itr;
    } 

    m_listConnPool.clear();

}


int SGConnsManager::Initialize()
{
    m_vcAllConns.clear();
    m_mpConnsIdxs.clear();
    m_listConnPool.clear();

    for (int i= 0; i < 100; i++)
    {
        SGGameConnector* pConn = new(g_pGameConnObjMng->CreateObjectMem()) SGGameConnector();
        if (pConn == NULL)
        {
            ERROR_LOG("new SGConnector() failed!\n");
            continue;
        }

        m_listConnPool.push_back(pConn);
    }
    return 0;
}

SGConnector* SGConnsManager::GetAConnObj()
{
    SGConnector* pNewConn = NULL;

    if ( !m_listConnPool.empty())
    {
        pNewConn = m_listConnPool.front();
        m_listConnPool.pop_front();
    }    
    
    if (pNewConn == NULL)
    {
        pNewConn = new(g_pGameConnObjMng->CreateObjectMem()) SGGameConnector();
        if (pNewConn == NULL)
        {
            ERROR_LOG("new SGConnector() failed\n");
            return NULL;
        }
    }

    pNewConn->OnNew();
    
    return pNewConn;
}


int SGConnsManager::AddNewConn(SGConnector* conn)
{
    int fd = conn->GetSocketFD();
    if (m_mpConnsIdxs.find(fd) != m_mpConnsIdxs.end())
    {
        ERROR_LOG("fd(%d) is existant\n",fd);
        return -1;
    }
    
    int idx = -1;
    for (int i = 0; i < (int)m_vcAllConns.size(); i++)
    {
        if (m_vcAllConns[i] == NULL)
        {
            idx = i;
            break;
        }
    }

    if (idx >= 0)
    {
        m_vcAllConns[idx] = conn;
    }else{
        m_vcAllConns.push_back(conn);
        idx = (int)m_vcAllConns.size() -1;
    }

    m_mpConnsIdxs[fd] = idx; 

    ANY_LOG("AddNewConn idx(%d)\n",idx);
    return idx;
}

int SGConnsManager::OnConnSocketFDChg(SGConnector* pConn, int oldFD)
{
    int newFD = pConn->GetSocketFD();
    if (oldFD == newFD || oldFD == -1)
    {
        return 0;
    }

    int idx = -1;
    for (int i = 0; i < (int)m_vcAllConns.size(); i++)
    {
        if (m_vcAllConns[i] == pConn)
        {
            idx = i;
            break;
        }
    }
    
    if (idx >= 0)
    {
        std::map<int,int>::iterator itr = m_mpConnsIdxs.find(oldFD);
        if (itr != m_mpConnsIdxs.end() && itr->second == idx)
        {
            m_mpConnsIdxs.erase(itr);
        }

        m_mpConnsIdxs[newFD] = idx;
    }else{
        AddNewConn(pConn);
    }

    return 0;
}

int SGConnsManager::RemoveConn(int fd)
{
    std::map<int,int>::iterator itr = m_mpConnsIdxs.find(fd);
    if (itr == m_mpConnsIdxs.end())
    {
        ERROR_LOG("Remove conn fd(%d) failed, not found!\n",fd);
        return -1;
    }

    int idx = itr->second;
    if (idx >= 0 && idx < (int)m_vcAllConns.size())
    {
        SGConnector* conn = m_vcAllConns[idx];
        conn->Close();
        //m_vcAllConns.erase(m_vcAllConns.begin() + idx);
        conn->Destroy();
        m_listConnPool.push_back(conn);
        m_vcAllConns[idx] = NULL;

        ANY_LOG("Remove conn idx(%d)\n",idx);
    }else{
        ERROR_LOG("Remove conn fd(%d) error idx(%d)\n",fd,idx);
    } 

    m_mpConnsIdxs.erase(itr);

    return 0;
}

int SGConnsManager::DestroyConn(SGConnector* pConn)
{
    if (pConn == NULL)
    {
        return -1;
    }
    
    pConn->Destroy();
    m_listConnPool.push_back(pConn);

    int iFD = pConn->GetSocketFD();
    if (iFD >= 0)
    {
        std::map<int,int>::iterator itr = m_mpConnsIdxs.find(iFD);
        if (itr != m_mpConnsIdxs.end())
        {
            int idx = itr->second;
            if (idx >= 0 && idx < (int)m_vcAllConns.size())
            {
                m_vcAllConns[idx] = NULL;

            }else{
                ERROR_LOG("Destroy conn error idx(%d) illegal!\n",idx);
            }

            m_mpConnsIdxs.erase(itr);
        }
        
    }

    return 0;
}

SGConnector* SGConnsManager::FindConn(int fd)
{
    std::map<int,int>::iterator itr = m_mpConnsIdxs.find(fd);
    if (itr == m_mpConnsIdxs.end())
    {
        return NULL;
    }

    int idx = itr->second;
    if (idx < 0 || idx >= (int)m_vcAllConns.size())
    {
        ERROR_LOG("data error %d %d\n",fd,idx);
        return NULL;
    }

    return m_vcAllConns[idx]; 
}

SGConnector* SGConnsManager::FindConnByIdx(int index)
{
    if (index < 0 || index >= (int)m_vcAllConns.size())
    {
       ERROR_LOG("data error %d\n",index); 
       return NULL;
    }

    return m_vcAllConns[index];
}







#include "SGLogicObjFactory.h"
//service conn manager
SGServiceConnsManager::SGServiceConnsManager()
{
}

SGServiceConnsManager::~SGServiceConnsManager()
{
    for (int i = 0; i < (int)m_vcAllConns.size(); i++)
    {
        SGConnector* pConn = g_pObjectFactory->GetConnector(m_vcAllConns[i].m_iObjMixID);
        if (pConn == NULL)
        {
            continue;
        }
        
        pConn->Close();
        g_pObjectFactory->DestroyConnector(m_vcAllConns[i].m_iObjMixID);
    }
    
    m_vcAllConns.clear();
}

int SGServiceConnsManager::Initialize()
{
    g_pMainCtl->AddRoutineCheck(SGServiceConnsManager::OnRoutineCheckConn, this, g_pLocalConfig->m_iServiceConnCheckIntvl);

    return 0;
}

int SGServiceConnsManager::AddNewConn(TObjMixID connID)
{
    struct timeval tvTime;
    gettimeofday(&tvTime,NULL);

    struct ServiceConnStatus stStatus;
    stStatus.m_iObjMixID = connID;
    stStatus.m_uiLastCheckTime = tvTime.tv_sec;
    
    m_vcAllConns.push_back(stStatus);

    ANY_LOG("AddNewConn idx(%u)\n",connID);
    return 0;
    
}

int SGServiceConnsManager::RemoveConn(TObjMixID connID)
{
    SGConnector* pConn = g_pObjectFactory->GetConnector(connID);
    if (pConn && pConn->IsConnected())
    {
        pConn->Close();
    }

    int idx = -1;
    for (int i = 0; i< (int)m_vcAllConns.size(); i++)
    {
        if (m_vcAllConns[i].m_iObjMixID == connID)
        {
            idx = i;
            m_vcAllConns.erase(m_vcAllConns.begin() + i);
            ANY_LOG("Remove conn idx(%u)\n",connID);
            break;
        } 
    }

    return 0;
}

int SGServiceConnsManager::CheckConns()
{
    int iCount = (int)m_vcAllConns.size();
    int endIdx = iCount; 
    
    struct timeval tvTime;
    gettimeofday(&tvTime,NULL);
    unsigned int uiNowTime = tvTime.tv_sec;

    struct ServiceConnStatus* pStatus = NULL;
    int i = 0;

    while( i < endIdx)
    {
        pStatus = &m_vcAllConns[i];
        if (uiNowTime - pStatus->m_uiLastCheckTime > (unsigned int)g_pLocalConfig->m_iServiceConnTimeout)
        {
           SGConnector* pConn = g_pObjectFactory->GetConnector(pStatus->m_iObjMixID);
            if (pConn && pConn->IsConnected())
            {
                pConn->Close();
                g_pObjectFactory->DestroyConnector(pStatus->m_iObjMixID);
            } 
            
            endIdx -= 1;
            m_vcAllConns[i] = m_vcAllConns[endIdx];
            ANY_LOG("check connection timeout:%d %u\n",pStatus->m_iObjMixID, uiNowTime - pStatus->m_uiLastCheckTime);
            continue;
        }

        i += 1; 
    }
    
    if (endIdx < iCount)
    {
        m_vcAllConns.erase(m_vcAllConns.begin() + endIdx, m_vcAllConns.end());
    }

    return 0;
}

int SGServiceConnsManager::OnHeartbeat(SGConnector* pConn)
{
    
    TObjMixID connID = CONSTRUCT_OBJECT_MIXID(pConn->GetObjectTag(), pConn->GetObjectID());

    ANY_LOG("------------------OnHeartbeat %u(%d)-----------------------------\n",connID,pConn->GetObjectID());
    
    struct timeval tvTime;
    gettimeofday(&tvTime,NULL);
    unsigned int uiNowTime = tvTime.tv_sec; 
    
    struct ServiceConnStatus* pStatus = NULL;
    for (int i = 0; i < (int)m_vcAllConns.size();i++)
    {
        pStatus = &m_vcAllConns[i];
        if (pStatus->m_iObjMixID == connID)
        {
            pStatus->m_uiLastCheckTime = uiNowTime;
            break;
        } 
    }

    return 0;
}

int SGServiceConnsManager::OnHandle(SGConnector* pConn, const char* pcData, int iLen)
{
    int cmd = *((int*)pcData);
    ANY_LOG("----------------------recv OnHandle Begin(%d)----------------------------\n",iLen);
    if (cmd == 1)
    {
        OnHeartbeat(pConn);
        SendMessage(pConn,1,"ok");
    }else{
        
    }

    ANY_LOG("-------------------------recv OnHandle end-------------------------------\n");
    return 0;
}

int SGServiceConnsManager::SendMsgToAll(int cmd,const char* pcData)
{
    return 0;
}

int SGServiceConnsManager::SendMessage(SGConnector* pConn,int cmd,const char* pcData)
{
    if (pConn == NULL)
    {
        return -1;
    }
    
    char szTemBuffer[MAX_SEND_MSG_LENGTH];
    int iPacketHeadLen = sizeof(int);
    int iCommandLen = sizeof(int);
    
    int iOffsetPos = 0 + iPacketHeadLen + iCommandLen;
    int iLen = strlen(pcData);
    if (MAX_SEND_MSG_LENGTH < iOffsetPos)
    {
       return -2; 
    }

    if (iLen > MAX_SEND_MSG_LENGTH -iOffsetPos)
    {
       iLen = MAX_SEND_MSG_LENGTH - iOffsetPos; 
    }

    memcpy(szTemBuffer + iOffsetPos,pcData,iLen);
    
    iOffsetPos -= iCommandLen;
    *((int*)(szTemBuffer + iOffsetPos)) = cmd;
    iLen += iCommandLen;
    
    iOffsetPos -= iPacketHeadLen;
    *((int*)(szTemBuffer + iOffsetPos)) = iLen;
    iLen += iPacketHeadLen;

    pConn->Send(szTemBuffer,iLen);
    return 0;
}

int SGServiceConnsManager::OnRoutineCheckConn(void* pUserData)
{
    SGServiceConnsManager* pThis = reinterpret_cast<SGServiceConnsManager*>(pUserData);
    
    pThis->CheckConns();
    
    return 0;
}
