#include "SGConfigReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>



SGConfigReader::SGConfigReader():m_isRead(false)
{
}

SGConfigReader::~SGConfigReader()
{
}

int SGConfigReader::ReadFile(const char* file)
{
   if (m_isRead)
   {
       Clear(); 
   }
   
    FILE* pFile = fopen(file,"rb");
    if (pFile == NULL)
    {
        return -1;
    }
    

    int iRet = fseek(pFile,0,SEEK_END);
    if (iRet)
    {
        fclose(pFile);
        return -2;
    }

    int iFileSize = ftell(pFile);
    if (iFileSize < 0)
    {
        fclose(pFile);
        return -3;
    } 

    iRet = fseek(pFile, 0, SEEK_SET);
    if (iRet)
    {
        fclose(pFile);
        return -4;
    }

    char* data = new char[iFileSize+1];
    memset(data,0,iFileSize + 1);

    iRet = fread(data, sizeof(char),iFileSize, pFile);
    if (iRet != iFileSize)
    {
        delete data;
        data = NULL;
        
        fclose(pFile);
        return -5;
    }
    
    m_isRead = true;

    iRet = ParseIni(data);

    delete data;
    data = NULL;

    fclose(pFile);
   
   return 0; 
}

int SGConfigReader::Clear()
{
    m_isRead = false;
    m_vcConfs.clear();

    return 0;
}

#define IS_SPACE(x) (x == ' ' || x == '\t')

int SGConfigReader::ParseIni(const char* data)
{
    if (data == NULL)
    {
        return -1;
    }

    const char* pTmp = data;
    std::string line;
    std::string section;

    while( *pTmp != '\0')
    {
        if (IS_SPACE(*pTmp))
        {
            pTmp++;
            continue;
        }        

        if (*pTmp != '\n')
        {
            line.push_back(*pTmp++);
            continue;

        }else if (!line.empty()){

            int len = (int)line.size();

            if (len > 0 && line[0] == '#')
            {
                line.clear();
                pTmp++;
                continue;
            } 

            if (len > 2 && line[0] == '[' && line[len -1] == ']')
            {
                section = line.substr(1,len - 2);

            }else{

                size_t pos = line.find('=');
                if (pos == std::string::npos || pos == 0 || (int)pos == len -1)
                {
                    line.clear();
                    pTmp++;
                    continue;
                } 

                std::string left = line.substr(0,pos);
                std::string right = line.substr(pos + 1,len - pos -1);

                struct ElemInfo elem;
                memset(&elem,0,sizeof(struct ElemInfo));

                strncpy(elem.section,section.c_str(),sizeof(elem.section) - 1);
                strncpy(elem.left,left.c_str(),sizeof(elem.left) - 1);
                strncpy(elem.right,right.c_str(),sizeof(elem.right) - 1);

                m_vcConfs.push_back(elem);
            }

            line.clear();
        }

        pTmp++;
    }

    return 0;
}

int SGConfigReader::GetKeyValue(const char* sec, const char* key, int& value, int defaultVal)
{
    if (!m_isRead)
    {
        return -1;
    }

    if (key == NULL)
    {
        return -2;
    }

    for (std::vector<struct ElemInfo>::iterator itr = m_vcConfs.begin(); itr != m_vcConfs.end(); itr++)
    {
        size_t len = strlen(itr->section);
        if (sec != NULL && (len != strlen(sec) || strncmp(sec,itr->section,len) != 0)) 
        {
            continue; 
        }

        len = strlen(itr->left);
        if (len == strlen(key) && strncmp(key,itr->left,len) == 0)
        {
            value = atoi(itr->right);
            return 0;
        }
    }

    value = defaultVal;

    return 0;
}

int SGConfigReader::GetKeyValue(const char* sec, const char* key, float& value, float defaultVal)
{
    if (!m_isRead)
    {
        return -1;
    }

    if (key == NULL)
    {
        return -2;
    }

    for (std::vector<struct ElemInfo>::iterator itr = m_vcConfs.begin(); itr != m_vcConfs.end(); itr++)
    {
        size_t len = strlen(itr->section);
        if (sec != NULL && (len != strlen(sec) || strncmp(sec,itr->section,len) != 0)) 
        {
            continue; 
        }

        len = strlen(itr->left);
        if (len == strlen(key) && strncmp(key,itr->left,len) == 0)
        {
            value = atof(itr->right);
            return 0;
        }
    }

    value = defaultVal;

    return 0;
}


int SGConfigReader::GetKeyValue(const char* sec, const char* key, char* value, size_t n,const char* defaultVal)
{
    if (!m_isRead)
    {
        return -1;
    }
    
    if (key == NULL)
    {
        return -2;
    } 

    for (std::vector<struct ElemInfo>::iterator itr = m_vcConfs.begin(); itr != m_vcConfs.end(); itr++)
    {
        size_t len = strlen(itr->section);
        if (sec != NULL &&( len != strlen(sec) || strncmp(sec, itr->section,len) != 0))
        {
            continue;
        }

        len = strlen(itr->left);
        if (len == strlen(key) && strncmp(key,itr->left,len) == 0)
        {
            strncpy(value,itr->right,n-1);
            value[n -1] = 0;
            return 0;
        }
    }

    if (defaultVal != NULL)
    {
        strncpy(value,defaultVal,n -1);
        value[n-1] = 0;
    }

    return 0;
}
