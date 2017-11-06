#ifndef __SGLOGTRACE_H
#define __SGLOGTRACE_H
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "SGSingleton.h"


#define MAX_FILENAME_LENGTH 255
#define MAX_DATE_LENGTH 30

class SGLogTrace{
   public:
    SGLogTrace(){
       strncpy(m_szPath,"../log/server/",sizeof(m_szPath) -1);
       strncpy(m_szBaseName,"server",sizeof(m_szBaseName) -1);
       strncpy(m_szExtName,".log",sizeof(m_szExtName) -1);
       m_szCurrLogName[0] = 0;

       Initialize();
    }
    int Initialize();
    int CheckPath();
    int WriteLog(const char* format,...);
    int WriteLogEx(const char* file, int line, const char* func,const char* format,...);
    void GetRealLogName(char* szRealName, int iNameSize);

    static void GetStringNowDate(char* szName, int iNameSize);
    static void GetStringTime(timeval tvTime,char* szStrTime, int iStrSize);
    static void AppendDate(char* szName, int iNameSize);

    protected:
    char m_szPath[MAX_FILENAME_LENGTH];
    char m_szBaseName[MAX_FILENAME_LENGTH];
    char m_szExtName[MAX_FILENAME_LENGTH];
    char m_szCurrLogName[MAX_FILENAME_LENGTH];
};

typedef SGSingleton<SGLogTrace> SGLogTraceSgl;
#define ANY_LOG(_FORMAT_STR,...) SGLogTraceSgl::Instance()->WriteLog(_FORMAT_STR,##__VA_ARGS__)

class SGErrLogTrace: public SGLogTrace{
    public:
        SGErrLogTrace():SGLogTrace(){
            strncpy(m_szBaseName,"error",sizeof(m_szBaseName) -1);
        }
};

typedef SGSingleton<SGErrLogTrace> SGErrLogTraceSgl;
#define ERROR_LOG(_FORMAT_STR,...) SGErrLogTraceSgl::Instance()->WriteLogEx(__FILE__,__LINE__,__FUNCTION__,_FORMAT_STR,##__VA_ARGS__)
#endif
