#include "SGServiceConnector.h"
#include "SGLogicObjFactory.h"
#include "SGConnsManager.h"

SGServiceConnector::SGServiceConnector()
{
}


SGServiceConnector::~SGServiceConnector()
{
}


int SGServiceConnector::GetObjectTag()
{
    return OBJECT_TAG_SERVICECONNECTOR;
}


int SGServiceConnector::OnHandle(const char* pcData, int iLen)
{
    return g_pConnsManager->OnHandle(this,pcData,iLen);
}
