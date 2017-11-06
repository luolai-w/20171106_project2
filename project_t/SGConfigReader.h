#ifndef __SGCONFIGREADER_H
#define __SGCONFIGREADER_H

#include <vector>

#define PARAM_NAME_LEN 128 

struct ElemInfo{
    char section[PARAM_NAME_LEN];
    char left[PARAM_NAME_LEN];
    char right[PARAM_NAME_LEN];
};


class SGConfigReader{
    public:
        SGConfigReader();
        ~SGConfigReader();                              
        int ReadFile(const char* fileName);
        int Clear();
        int GetKeyValue(const char* sec, const char* key, int &value, int defaultVal = 0); 
        int GetKeyValue(const char* sec, const char* key, float& value, float defaultVal = 0.0f);
        int GetKeyValue(const char* sec, const char* key, char* value, size_t n,const char* defaultVal = NULL);
    protected:
        int ParseIni(const char* data);

    private:
       bool m_isRead;
       std::vector<struct ElemInfo> m_vcConfs; 
       
};


#endif
