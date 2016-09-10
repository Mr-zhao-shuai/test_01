/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */

#include "fcgi_config.h"

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"
//#include "keymnglog.h"
#include "string.h"
#include "make_log.h"
#include "data_op.h"


#define FILE_NAME_LOG_DATA_OP 								"data_op"    //定义Log文件夹的名字
#define PRE_DES_LOG_DATA_OP   								"data_op"    //定义Log文件的前缀描述

int main ()
{
    int 				ret = 0;
    
    char *ret_p = NULL;
    
    char 				query_cmd_cmd[64] = {0};        //query命令中cmd参数
    char 				query_cmd_fromId[32] = {0};     //query命令中fromId参数
    char 				query_cmd_count[32] = {0};      //query命令中count参数
    //char 				query_cmd_user[128] = {0};       //query命令中user参数
    
    LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "[INFO] start");
    while (FCGI_Accept() >= 0) {
        char *query_string = getenv("QUERY_STRING");     //网页请求的命令


        printf("Content-type: text/html\r\n");
        printf("\r\n");
            
        if (query_string != NULL) {
        	  /*提取命令属性  cmd  fromID   count  user */  
        	  if ( (query_string_value_get(query_string, "cmd", query_cmd_cmd, NULL) ) == 1 ) {
        	  	  LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "[INFO] cmd not found");
        	  } else {
        	      LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "[INFO] cmd [%s]", query_cmd_cmd);
        	  } 	  
        }
        else {

        }
          
        if (strcmp(query_cmd_cmd, "newFile") == 0) {
            query_string_value_get(query_string, "fromId", query_cmd_fromId, NULL);
            query_string_value_get(query_string, "count", query_cmd_count, NULL);
            //query_string_value_get(query_string, "kind", kind, NULL);
            LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "=== fromId:%s, count:%s", query_cmd_fromId, query_cmd_count);
            //cgi_init();



            ret_p = file_list_to_json_print(atoi(query_cmd_fromId), atoi(query_cmd_count), query_cmd_cmd, NULL);
            printf("%s", ret_p);
            free(ret_p);
        }

        /*************************提取数据将其转换成json格式字符串 并发送给前端*********************************/





    } /* while */
//END:
 
    return ret;
}








