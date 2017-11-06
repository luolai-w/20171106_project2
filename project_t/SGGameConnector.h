#ifndef __SGGAMECONNECTOR_H
#define __SGGAMECONNECTOR_H

#include "SGConnector.h"

class SGGameConnector:public SGConnector{
    public:
        SGGameConnector();
        ~SGGameConnector();

        int OnHandle(const char* pcData, int iLen);
        int GetObjectTag();
    private:
         
};

#endif
