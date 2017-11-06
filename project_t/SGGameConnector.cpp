#include "SGGameConnector.h"
#include "SGLogicObjFactory.h"


SGGameConnector::SGGameConnector()
{
}


SGGameConnector::~SGGameConnector()
{
    ERROR_LOG("destruct SGGameConnector %d\n",GetObjectID());//test
}

int SGGameConnector::GetObjectTag()
{
   return OBJECT_TAG_GAMECONNECTOR; 
}

int SGGameConnector::OnHandle(const char* pcData, int iLen)
{
    return SGConnector::OnHandle(pcData,iLen);
}
