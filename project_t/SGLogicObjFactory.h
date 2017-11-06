#ifndef __SGLOGICOBJFACTORY_H
#define __SGLOGICOBJFACTORY_H

#include "SGObjectPool.h"
#include "SGSingleton.h"
#include "SGIObjectMng.h"
#include "SGGameConnector.h"
#include "SGServiceConnector.h"
#include "SGListener.h"
#include "SGEpollEventLoop.h"


typedef SGIObjectMng<SGServiceConnector> SGServiceConnObjMng;
typedef SGSingleton<SGServiceConnObjMng> SGServiceConnObjMngSgl;
#define g_pServiceConnObjMng SGServiceConnObjMngSgl::Instance()

typedef SGIObjectMng<SGGameConnector> SGGameConnObjMng;
typedef SGSingleton<SGGameConnObjMng> SGGameConnObjMngSgl;
#define g_pGameConnObjMng SGGameConnObjMngSgl::Instance()

typedef SGIObjectMng<SGListener> SGListenerObjMng;
typedef SGSingleton<SGListenerObjMng> SGListenerObjMngSgl;
#define g_pListenerObjMng SGListenerObjMngSgl::Instance()



class SGLogicObjFactory{
    public:
        SGLogicObjFactory();
        ~SGLogicObjFactory();
        
        int Initialize();
        int InitEventLoop();

        SGEpollEventLoop* GetEventLoop(){
            return m_pEventLoop;
        }
        bool IsListener(TObjMixID iObjMixID);
        SGListener* GetListener(TObjMixID iObjMixID);
        
        SGConnector* GetConnector(TObjMixID iObjMixID);
        int DestroyConnector(TObjMixID iObjMixID);

    private:
        SGEpollEventLoop* m_pEventLoop;
        SGListener* m_pListener;
};

typedef SGSingleton<SGLogicObjFactory> SGLogicObjFactorySgl;
#define g_pObjectFactory SGLogicObjFactorySgl::Instance()

#endif
