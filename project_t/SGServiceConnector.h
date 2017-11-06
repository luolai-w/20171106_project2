#ifndef __SGSERVICECONNECTOR_H
#define __SGSERVICECONNECTOR_H
#include "SGConnector.h"

class SGServiceConnector:public SGConnector{
    public:
        SGServiceConnector();
        ~SGServiceConnector();

        int OnHandle(const char* pcData, int iLen);
        int GetObjectTag();
    private:
};

#endif
