#ifndef __SGOBJECTPOOL_H
#define __SGOBJECTPOOL_H

#include <vector>
#include "SGLogTrace.h"

class SGObject;
class SGObjectCreator;

struct ObjectInfo{
    int m_iPrevIdx;
    int m_iNextIdx;
    
    //data
    bool m_isBusy;
    SGObject* m_pDataObj; 
};

typedef unsigned int TObjMixID;

#define CONSTRUCT_OBJECT_MIXID(iObjTag,iObjID) ((((unsigned int)(iObjTag)) << 24) | ((unsigned int)(iObjID)))
#define DESTRUCT_OBJECT_MIXID(iObjMixID,iObjTag, iObjID)    \
do{\
    iObjTag = ((unsigned int)(iObjMixID)) >> 24;\
    iObjID = ((unsigned int)(iObjMixID)) & 0x00FFFFFF ;\
}while(0)

enum EnObjectTag{
   OBJECT_TAG_GAMECONNECTOR = 1,
   OBJECT_TAG_SERVICECONNECTOR,
   OBJECT_TAG_LISTENER, 
};


class SGObjectCreator{
    public:
        SGObjectCreator(size_t nObjSize, int iInitCount,int iStepCount);
        ~SGObjectCreator();

        int Initialize();
        SGObject* GetObject(int idx);
        SGObject* CreateObject();
        int DestroyObject(int idx);
    //protected:
        int MakeObjects(int iCount);
    private:
        size_t m_nObjSize;
        int m_iInitCount;
        int m_iStepCount;
        int m_iCurrCount;

        int m_iBusyHead;
        int m_iBusyCount;
        int m_iFreeHead;
        int m_iFreeCount;
        std::vector<struct ObjectInfo> m_vcAllObjects;
};

class SGObject{

    public:
        SGObject(){}
        virtual ~SGObject(){
            ERROR_LOG("destruct SGObject %d\n",m_iObjectID);//test
        }
    
        virtual int Initialize() = 0;
        static void* operator new(size_t nSize) throw();
        static void  operator delete(void* pMem);
        static void* operator new(size_t nSize, void* pMem) throw();
        int GetObjectID(){
            return m_iObjectID; 
        }
    private:
        int m_iObjectID;
        friend int SGObjectCreator::MakeObjects(int);

};

#endif
