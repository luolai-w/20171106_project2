#include "SGLogicObjFactory.h"
#include "SGLogTrace.h"


SGLogicObjFactory::SGLogicObjFactory():m_pEventLoop(NULL),
m_pListener(NULL)
{
}

SGLogicObjFactory::~SGLogicObjFactory()
{
    if (m_pEventLoop != NULL)
    {
        delete m_pEventLoop;
        m_pEventLoop = NULL;
    }

    if (m_pListener != NULL)
    {
        g_pListenerObjMng->DestroyObjectByID(m_pListener->GetObjectID());
        m_pListener = NULL;
    }
}


int SGLogicObjFactory::Initialize()
{
    int iRet = g_pServiceConnObjMng->Initialize(0,1);
    if (iRet)
    {
        ERROR_LOG("g_pServiceConnObjMng->Initialize(0,1) failed:%d\n",iRet);
        return -2;
    }

    iRet = g_pGameConnObjMng->Initialize(0,1);
    if (iRet)
    {
        ERROR_LOG("g_pGameConnOjbMng->Initialize(0,1) failed:%d\n",iRet);
        return -3;
    }

    iRet = g_pListenerObjMng->Initialize(0,1);
    if (iRet)
    {
        ERROR_LOG("g_pListenerObjMng->Initialize(0,1) failed:%d\n",iRet);
        return -4;
    }


    iRet = InitEventLoop(); 
    if (iRet)
    {
        ERROR_LOG("InitEventLoop failed:%d\n",iRet);    
        return -5;
    }

    m_pListener = new(g_pListenerObjMng->CreateObjectMem()) SGListener(m_pEventLoop);
    if (m_pListener == NULL)
    {
        ERROR_LOG("new SGListener failed\n");
        return -6;
    }
    
    return 0;
}

int SGLogicObjFactory::InitEventLoop()
{

    m_pEventLoop = new SGEpollEventLoop(100 * 10000);
    if (m_pEventLoop == NULL)
    {
        ERROR_LOG("new SGEpollEventLoop() failed\n");
        return -1;
    }
    
    int iRet = m_pEventLoop->Initialize();
    if (iRet)
    {
        ERROR_LOG("m_pEventLoop->Initialize() failed:%d\n",iRet);
        return -2;
    } 

    return 0;
}

bool SGLogicObjFactory::IsListener(TObjMixID iObjMixID)
{
    int iObjTag, iObjID;

    DESTRUCT_OBJECT_MIXID(iObjMixID,iObjTag,iObjID);
    
    switch(iObjTag)
    {
        case OBJECT_TAG_LISTENER: return true;
        default:break;
    }
    return false;
}

SGListener* SGLogicObjFactory::GetListener(TObjMixID iObjMixID)
{
    int iObjTag, iObjID;
    
    DESTRUCT_OBJECT_MIXID(iObjMixID,iObjTag,iObjID);
    
    switch(iObjTag)
    {
        case OBJECT_TAG_LISTENER:
            return m_pListener; 
        default:break;
    }

    return NULL;
}

SGConnector* SGLogicObjFactory::GetConnector(TObjMixID iObjMixID)
{
    int iObjTag, iObjID;

    DESTRUCT_OBJECT_MIXID(iObjMixID,iObjTag,iObjID);
    
    switch( iObjTag)
    {
        case OBJECT_TAG_GAMECONNECTOR:
            return g_pGameConnObjMng->GetObjectByID(iObjID);
        case OBJECT_TAG_SERVICECONNECTOR:
            return g_pServiceConnObjMng->GetObjectByID(iObjID);
        default:break;
    }

    return NULL;
}

int SGLogicObjFactory::DestroyConnector(TObjMixID iObjMixID)
{
    int iObjTag, iObjID;
    
    DESTRUCT_OBJECT_MIXID(iObjMixID, iObjTag,iObjID);
    
    switch(iObjTag)
    {
        case OBJECT_TAG_SERVICECONNECTOR:
            g_pServiceConnObjMng->DestroyObjectByID(iObjID);
            break;
        default:break;
    }

    return 0;  
}
