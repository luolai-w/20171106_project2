#include <cstdio>
#include <mysql.h>
#include <cstring>

static MYSQL* g_pMysql = NULL;

const char* g_pcHost = "127.0.0.1";
const char* g_pcUser = "root";
const char* g_pcPW = "haoxin,!@#";
const char* g_pcDBName = "llw_sgc_test_2004";

int main()
{

    g_pMysql = mysql_init(NULL);

    if (g_pMysql == NULL)
    {
        fprintf(stderr,"mysql_init failed: %s\n",mysql_error(g_pMysql));
        return -1;
    }

    mysql_options(g_pMysql,MYSQL_SET_CHARSET_NAME,MYSQL_AUTODETECT_CHARSET_NAME);
    if (mysql_real_connect(g_pMysql,g_pcHost,g_pcUser,g_pcPW,g_pcDBName,3306,NULL,0) == NULL)
    {
        fprintf(stderr,"mysql_real_connect failed: %s\n",mysql_error(g_pMysql));
        return -2;
    }

    const char* pcSql = "select uid,manor_level,account,(uid>>17)svrid,user_name from user where manor_level >= 10 ";
    const char* pcUpdateSql = "update user set manor_level = 10 where manor_level > 10 ";
 
    if (mysql_real_query(g_pMysql,pcSql,strlen(pcSql)))
    {
       fprintf(stderr,"mysql_real_query failed: %s\n",mysql_error(g_pMysql)); 
       return -3;
    }

    MYSQL_RES* pMysqlRes = mysql_store_result(g_pMysql);

    if (pMysqlRes != NULL)
    {
        unsigned int nNumRows = mysql_num_rows(pMysqlRes);
        unsigned int nNumFields = mysql_num_fields(pMysqlRes);
        //retrieve rows
        fprintf(stdout,"fields = %u, rows = %u\n",nNumFields,nNumRows);
        
        MYSQL_FIELD* pcFieldDesc = mysql_fetch_fields(pMysqlRes);
        if (pcFieldDesc == NULL)
        {
            fprintf(stderr,"mysql_fetch_fields failed: %s\n",mysql_error(g_pMysql));
            return -4;
        }

        for (int i = 0; i < (int)nNumFields; i++)
        { 
            int len = pcFieldDesc[i].max_length > pcFieldDesc[i].name_length ? pcFieldDesc[i].max_length:pcFieldDesc[i].name_length;
            fprintf(stdout,"[%*s] ",len,pcFieldDesc[i].name);
        }

        //for (int i = 0; i < (int)nNumFields; i++)
        //{ 
        //    fprintf(stdout,"\nname=%s, length = %u, max_length = %u, name_length = %u,table_length = %u",pcFieldDesc[i].name,pcFieldDesc[i].length,pcFieldDesc[i].max_length,pcFieldDesc[i].name_length,pcFieldDesc[i].table_length);
        //}

        fprintf(stdout,"\n");
        
        MYSQL_ROW row = NULL;
        while((row = mysql_fetch_row(pMysqlRes)))
        {
            for (int i = 0; i < (int)nNumFields; i++)
            {
                int len = pcFieldDesc[i].max_length > pcFieldDesc[i].name_length ? pcFieldDesc[i].max_length:pcFieldDesc[i].name_length;
                fprintf(stdout,"[%*s] ", len,row[i]);
            }
            fprintf(stdout,"\n");
        }

        mysql_free_result(pMysqlRes);
    }else{
        if (mysql_field_count(g_pMysql) == 0)
        {
            unsigned int nNumRowsAffected = mysql_affected_rows(g_pMysql);
            fprintf(stdout,"affected rows: %u\n",nNumRowsAffected);
        }else{
            fprintf(stderr,"mysql_store_result failed: %s\n",mysql_error(g_pMysql));
        }
    }

    mysql_close(g_pMysql);
    g_pMysql = NULL;

    return 0;
}
