#ifndef __SGIOBJECTMNG_H
#define __SGIOBJECTMNG_H
#include "SGObjectPool.h"

template<typename T>
class SGIObjectMng{
    public:
        SGIObjectMng():m_pObjCreator(NULL){}
        ~SGIObjectMng(){
            if (m_pObjCreator != NULL)
            {
                delete m_pObjCreator;
                m_pObjCreator = NULL;
            }
        }

        int Initialize(int iInitCount,int iStepCount);
        T* GetObjectByID(int id);
        int DestroyObjectByID(int id);
    
        void* CreateObjectMem();
    private:
        SGObjectCreator* m_pObjCreator;
        
};


template<typename T>
int SGIObjectMng<T>::Initialize(int iInitCount, int iStepCount)
{
    m_pObjCreator = new SGObjectCreator(sizeof(T),iInitCount,iStepCount);
    if (m_pObjCreator == NULL)
    {
        return -1;
    }

    return 0;
}

template<typename T>
T* SGIObjectMng<T>::GetObjectByID(int id)
{
    
    return (T*)m_pObjCreator->GetObject(id);
}

template<typename T>
void* SGIObjectMng<T>::CreateObjectMem()
{
    if (m_pObjCreator == NULL)
    {
        return NULL;
    }

    SGObject* pObj = m_pObjCreator->CreateObject();

    return reinterpret_cast<void*>(pObj);
}

template<typename T>
int SGIObjectMng<T>::DestroyObjectByID(int id)
{
    SGObject* pObj = m_pObjCreator->GetObject(id);
    if (pObj == NULL)
    {
        return -1;
    }

    pObj->~SGObject();

    int iRet =  m_pObjCreator->DestroyObject(id);

    return iRet;
}

#endif
