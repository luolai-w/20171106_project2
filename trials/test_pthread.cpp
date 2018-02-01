#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "SGDBImplThread.h"

int main(void)
{
   SGDBImplThread threads[2];

   int ret = 0;
   for (int i = 0; i < 2; i++)
   {
        ret = threads[i].Initialize();
        if (ret)
        {
            fprintf(stderr,"thread %d initialize failed(%d)\n",i,ret);
            break;
        }

        ret = threads[i].Start();
        if (ret)
        {
            fprintf(stderr,"thread %d start failed(%d)\n",i,ret);
            break;
        }
   }  


    if (ret)
    {
        for (int i = 0; i< 2;i++)
        {
            threads[i].Stop();
        }
        return -1;
    }


    struct timeval tm;
    
    int c = 0;
    while (c < 10)
    {
       gettimeofday(&tm,NULL);
       fprintf(stdout,"%d main thread> hello world!\n",tm.tv_sec); 
       c++;
       sleep(5);
    }

    //停止所有线程
    for (int i = 0; i < 2; i++)
    {
        ret = threads[i].Stop();
        if (ret)
        {
            fprintf(stderr,"thread %d stop failed(%d)\n",i,ret);
        }
    }

    //等待所有线程退出
    for (int i = 0; i < 2; i++)
    {
        threads[i].Join();
    }

    return 0;
}
