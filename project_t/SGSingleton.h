#ifndef __SGSINGLETON_H
#define __SGSINGLETON_H
#include <stdio.h>
#include <stdlib.h>



template <class CLASSTYPE>
class SGSingleton{
    public:
    SGSingleton(){}
    
    static CLASSTYPE* Instance()
    {
        if(m_pSingleton == NULL)
        {
            m_pSingleton = new SGSingleton<CLASSTYPE>();
        }
        return &m_pSingleton->m_stInstance;
    }
    private:
    CLASSTYPE m_stInstance;
    static SGSingleton<CLASSTYPE>* m_pSingleton;
};

template <class CLASSTYPE>
SGSingleton<CLASSTYPE>* SGSingleton<CLASSTYPE>::m_pSingleton = NULL;
#endif
