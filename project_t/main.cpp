#include <string.h>
#include <stdlib.h>
#include "SGMainCtl.h"
#include <unistd.h>
#include <sys/types.h>

SGMainCtl* g_pMainCtl = NULL;

int main(int args, char** argv)
{
    struct SvrArguments stSvrArgs;
    memset(&stSvrArgs,0,sizeof(struct SvrArguments));
    strncpy(stSvrArgs.m_acSvrName,argv[0],sizeof(stSvrArgs.m_acSvrName) -1);

    g_pMainCtl = new SGMainCtl(stSvrArgs);
    if (g_pMainCtl == NULL)
    {
        fprintf(stderr,"new SGMainCtl failed!\n");
        exit(1);
    }

    if (fork() == 0)
    {
        int ret = g_pMainCtl->Initialize();
        if (ret)
        {
            
            fprintf(stderr,"error got:%d\n",ret);

            delete g_pMainCtl;
            g_pMainCtl = NULL;

            exit(1); 
        } 

        g_pMainCtl->Run();
    }else{
    }

    delete g_pMainCtl;
    g_pMainCtl = NULL;

    return 0;
}
