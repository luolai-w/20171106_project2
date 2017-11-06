#include "SGObjectPool.h"
#include "SGLogTrace.h"




void* SGObject::operator new(size_t nsize) throw()
{
    return NULL;
}

void SGObject::operator delete(void* pMem)
{
    return ;
}

void* SGObject::operator new(size_t nSize, void* pMem) throw()
{
    return pMem;
}


SGObjectCreator::SGObjectCreator(size_t nObjSize, int iInitCount, int iStepCount):m_nObjSize(nObjSize),
m_iInitCount(iInitCount),
m_iStepCount(iStepCount),
m_iCurrCount(0),
m_iBusyHead(-1),
m_iBusyCount(0),
m_iFreeHead(-1),
m_iFreeCount(0)
{
   Initialize(); 
}

SGObjectCreator::~SGObjectCreator()
{
    if (m_iInitCount > 0)
    {
        delete[] reinterpret_cast<char*>(m_vcAllObjects[0].m_pDataObj);
    }    

    for (int i = m_iInitCount; i < m_iCurrCount; i += m_iStepCount)
    {
        delete[] reinterpret_cast<char*>(m_vcAllObjects[i].m_pDataObj);
    }
}

int SGObjectCreator::Initialize()
{

    MakeObjects(m_iInitCount);

    return 0;
}


SGObject* SGObjectCreator::GetObject(int idx)
{
    SGObject* pObj = NULL;

    if ( idx >= 0 && idx < m_iCurrCount) 
    {
        struct ObjectInfo* info = &m_vcAllObjects[idx];
        if (!info->m_isBusy)
        {
            return NULL;        
        }        
        pObj = info->m_pDataObj;
    }

    return pObj;
}

SGObject* SGObjectCreator::CreateObject()
{
    SGObject* pNewObj = NULL;

    if (m_iFreeCount <= 0)
    {
        MakeObjects(m_iStepCount);
    }

    if (m_iFreeHead == -1)
    {
        return NULL;
    }
    
    int iCurrIdx = m_iFreeHead;

    struct ObjectInfo* pObjInfo = &m_vcAllObjects[iCurrIdx];

    m_iFreeHead = pObjInfo->m_iNextIdx;
    m_iFreeCount -= 1; 

    if (m_iFreeHead != -1)
    {
       m_vcAllObjects[m_iFreeHead].m_iPrevIdx = -1; 
    }


    pObjInfo->m_iPrevIdx = -1;
    pObjInfo->m_iNextIdx = m_iBusyHead;
    pObjInfo->m_isBusy = true;
    m_iBusyCount += 1;
    
    if (m_iBusyHead != -1)
    {
        m_vcAllObjects[m_iBusyHead].m_iPrevIdx = iCurrIdx;
    }
    m_iBusyHead = iCurrIdx;


    pNewObj = pObjInfo->m_pDataObj;

    return pNewObj;
}

int SGObjectCreator::MakeObjects(int iCount)
{
    if (iCount <= 0)
    {
        return 0;
    }

    size_t nSumSize = iCount*m_nObjSize;

    char* pcMem = new char[nSumSize];
    if (pcMem == NULL)
    {
        return -1;
    }
    
    for ( int i = 0; i < iCount; i++)
    {
        struct ObjectInfo stObjInfo;
        int idx = i + m_iCurrCount;

        stObjInfo.m_iPrevIdx = -1;
        stObjInfo.m_iNextIdx = m_iFreeHead;
        
        if (m_iFreeHead != -1)
        {
            m_vcAllObjects[m_iFreeHead].m_iPrevIdx = idx;
        }
        m_iFreeHead = idx;

        stObjInfo.m_isBusy = false;
        stObjInfo.m_pDataObj = reinterpret_cast<SGObject*>(pcMem + i*m_nObjSize);
        (stObjInfo.m_pDataObj)->m_iObjectID = idx;

        
        m_vcAllObjects.push_back(stObjInfo);
        
    }

    m_iCurrCount += iCount;
    m_iFreeCount += iCount;
    
   return 0; 
}


int SGObjectCreator::DestroyObject(int idx)
{
    if ( idx < 0 || idx >= m_iCurrCount)
    {
        return -1;
    }

    struct ObjectInfo* stObjInfo = &m_vcAllObjects[idx];
    
    if (stObjInfo->m_iPrevIdx == -1)
    {
        m_iBusyHead = stObjInfo->m_iNextIdx;
    }else{
        m_vcAllObjects[stObjInfo->m_iPrevIdx].m_iNextIdx = stObjInfo->m_iNextIdx;
    }      
 
    if (stObjInfo->m_iNextIdx != -1)
    {
        m_vcAllObjects[stObjInfo->m_iNextIdx].m_iPrevIdx = stObjInfo->m_iPrevIdx;
    }

    m_iBusyCount -= 1;

    stObjInfo->m_iPrevIdx = -1;
    stObjInfo->m_iNextIdx = m_iFreeHead;
    
    if (m_iFreeHead != -1)
    {
        m_vcAllObjects[m_iFreeHead].m_iPrevIdx = idx;
    }
    m_iFreeHead = idx;

    stObjInfo->m_isBusy = false;
    m_iFreeCount += 1;

    return 0;
}
