#include "SGLogTrace.h"


int SGLogTrace::Initialize()
{
   return CheckPath();
}

int SGLogTrace::CheckPath()
{
    m_szPath[MAX_FILENAME_LENGTH -1] = 0;

    int iLen = strlen(m_szPath);
    if (m_szPath[iLen -1] != '/')
    {
        strncat(m_szPath,"/",MAX_FILENAME_LENGTH - 1);
    }     

   char aszPath[MAX_FILENAME_LENGTH] = {0};
   strncpy(aszPath,m_szPath,MAX_FILENAME_LENGTH -1);
   char* pcTempPath = aszPath;

   if(*pcTempPath == '/') ++pcTempPath;
     
   while(*pcTempPath)
    {
        if(*pcTempPath != '/')
        {
            pcTempPath++;
            continue;
        }
        *pcTempPath = '\0';
        if(access(aszPath,F_OK))
        {
           if(mkdir(aszPath,S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
            {
                return -1;
            } 
        }
        *pcTempPath++ = '/';
    }
    
    return 0;
} 


int SGLogTrace::WriteLog(const char* format,...)
{
    GetRealLogName(m_szCurrLogName,MAX_FILENAME_LENGTH);

    FILE* pFile = fopen(m_szCurrLogName,"a+");
    if (!pFile)
    {
        return -2;
    } 
    
    struct timeval tvTime;
    gettimeofday(&tvTime,NULL);
    char currTimeString[255] = {0};
    GetStringTime(tvTime,currTimeString,sizeof(currTimeString));
    fprintf(pFile,"[%s] ",currTimeString);
    
    va_list ap;
    va_start(ap,format);
    vfprintf(pFile,format,ap);
    va_end(ap);

    fflush(pFile);
    fclose(pFile);

    return 0;    
}

int SGLogTrace::WriteLogEx(const char* file,int line,const char* func,const char* format,...)
{
    GetRealLogName(m_szCurrLogName,MAX_FILENAME_LENGTH);

    FILE* pFile = fopen(m_szCurrLogName,"a+");
    if (!pFile)
    {
        return -2;
    } 

    struct timeval tvTime;
    gettimeofday(&tvTime,NULL);
    char currTimeString[255] = {0};
    GetStringTime(tvTime,currTimeString,sizeof(currTimeString));
    fprintf(pFile,"[%s] ",currTimeString);

    fprintf(pFile,"<%s:%d %s> ",file,line,func);
    va_list ap;
    va_start(ap,format);
    vfprintf(pFile,format,ap);
    va_end(ap);

    fflush(pFile);
    fclose(pFile);

    return 0;    
}

void SGLogTrace::GetStringNowDate(char* szName, int iNameSize)
{
    if (szName == NULL || iNameSize <= 0 ) return;

    struct timeval tvTimeVal;
    gettimeofday(&tvTimeVal,NULL);
    struct tm stTemTm;
    struct tm* pTemTm = localtime_r(&tvTimeVal.tv_sec,&stTemTm);
    snprintf(szName, iNameSize,"%04d-%02d-%02d",pTemTm->tm_year + 1900, pTemTm->tm_mon + 1, pTemTm->tm_mday);
}

void SGLogTrace::GetStringTime(timeval tvTime,char* szStrTime, int iStrSize)
{
    if (szStrTime == NULL || iStrSize <= 0 ) return;

    struct tm stTemTm;
    struct tm* pTemTm = localtime_r(&tvTime.tv_sec,&stTemTm);
    snprintf(szStrTime, iStrSize,"%04d-%02d-%02d %02d:%02d:%02d.%06d",pTemTm->tm_year + 1900, pTemTm->tm_mon + 1, pTemTm->tm_mday,pTemTm->tm_hour, pTemTm->tm_min, pTemTm->tm_sec, static_cast<int>(tvTime.tv_usec));
}

void SGLogTrace::AppendDate(char* szName, int iNameSize)
{
    if (szName == NULL or iNameSize <= 0) return;
        
    char pszDate[MAX_DATE_LENGTH] = {0};
    GetStringNowDate(pszDate,MAX_DATE_LENGTH);

    snprintf(szName+strlen(szName), iNameSize, "_%s",pszDate);
}

void SGLogTrace::GetRealLogName(char* szRealName, int iNameSize)
{
    if(szRealName == NULL or iNameSize <= 0) return;

    char szDate[MAX_DATE_LENGTH] = {0};
    GetStringNowDate(szDate,MAX_DATE_LENGTH);
    
   snprintf(szRealName, iNameSize, "%s%s_%s%s",m_szPath,m_szBaseName,szDate,m_szExtName);
}
