#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>


#define MAX_FILENAME_LENGTH 255
#define MAX_DATE_LENGTH 30

int CheckPath(const char* path)
{
    if (path == NULL || path[0] == 0) return 0;
    char aszPath[MAX_FILENAME_LENGTH] = {0};
    strncpy(aszPath,path,MAX_FILENAME_LENGTH -1);
    //int iLen = strlen(aszPath);
    //if (aszPath[iLen -1] != '/')
    //{
    //    strncat(aszPath,"/",MAX_FILENAME_LENGTH - 1);
    //}     
    
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


int WriteFile(const char* szName, const char* format,...)
{
   if (CheckPath(szName))
    {
        return -1;
    }

    FILE* pFile = fopen(szName,"a+");
    if (!pFile)
    {
        return -2;
    } 

    va_list ap;
    va_start(ap,format);
    vfprintf(pFile,format,ap);
    va_end(ap);

    fflush(pFile);
    fclose(pFile);

    return 0;    
}

void GetStringNowDate(char* szName, int iNameSize)
{
    if (szName == NULL || iNameSize <= 0 ) return;

    struct timeval tvTimeVal;
    gettimeofday(&tvTimeVal,NULL);
    struct tm stTemTm;
    struct tm* pTemTm = localtime_r(&tvTimeVal.tv_sec,&stTemTm);
    snprintf(szName, iNameSize,"%04d-%02d-%02d",pTemTm->tm_year + 1900, pTemTm->tm_mon + 1, pTemTm->tm_mday);
}

void AppendDate(char* szName, int iNameSize)
{
    if (szName == NULL or iNameSize <= 0) return;
        
    char pszDate[MAX_DATE_LENGTH] = {0};
    GetStringNowDate(pszDate,MAX_DATE_LENGTH);

    snprintf(szName+strlen(szName), iNameSize, "_%s",pszDate);
}



int main(int argc, char** argv)
{
    char szPath[MAX_FILENAME_LENGTH] = "./log/test";
    char szContent[1024] = "this is for a test";
    if(argc >= 2)
    {
        strncpy(szPath,argv[1],MAX_FILENAME_LENGTH -1);
    } 

    if(argc >= 3)
    {
        strncpy(szContent,argv[2],1024 -1);
    }
    char szRealPath[MAX_FILENAME_LENGTH] = {0};

    strncpy(szRealPath,szPath,MAX_FILENAME_LENGTH);
    szRealPath[MAX_FILENAME_LENGTH -1] = 0;

    AppendDate(szRealPath, sizeof(szRealPath)/sizeof(szRealPath[0])); 

    WriteFile(szRealPath,"%s\n",szContent);

    return 0;
}
