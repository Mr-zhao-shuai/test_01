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
#include "echo_op.h"


#define MAX_BUF_SOURCE 							(102400)
#define MAX_RECV_FILE_NAME					(256)
#define MAX_FILE_URL_FDFS						(256)

int main ()
{
    int 				ret = 0;
    int 				i = 0;
    int 				ch = 0;

    int 				len = 0;                  		//网页内容长度
    int 				count = 0;    								//网页显示的计数值
    char 				*text_source = NULL;
    char 				file_name[MAX_RECV_FILE_NAME] = {0};    			//文件名字
    char 				file_id_fdfs[MAX_FILE_ID_FDFS] = {0};
    char 				file_url_fdfs[MAX_FILE_URL_FDFS] = {0};

    text_source = (char *)calloc(1, sizeof(char) * MAX_BUF_SOURCE);
    if (text_source == NULL) {
        LOG("11", "12", "[ERROR] malloc error");
        ret = -1;
        goto END;	
    }

    LOG("11", "12", "[INFO] start");
    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");

        printf("Content-type: text/html\r\n"
                "\r\n"
                "<title>FastCGI echo</title>"
                "<h1>FastCGI echo</h1>\n"
                "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
            LOG("11", "12", "[INFO] text_len= %d", len);
        }
        else {
            len = 0;
            LOG("11", "12", "[INFO] text_len= %d", len);
        }

        if (len <= 0) {
            LOG("11", "12", "[INFO] text_len= %d", len);
        }
        else {
        	    for (i = 0; i < len; i++) {
                 if ((ch = getchar()) < 0) {
                 LOG("11", "12", "[INFO] Not enough bytes received on standard input ");
                 goto END;
              }
              text_source[i] = ch;
              }
        }
            /**************************得到文件的內容 利用相同的文件名存储*********************************/
            if (fiel_source_get(text_source, len, file_name) != 0) {
                ret = -1;
                goto END;
            }
            LOG("11", "12", "[INFO] -----------filename= %s", file_name);

        /*************************上传文件  得到文件返回的存储文件名**********************************/
            if (recv_file_upload(file_name, file_id_fdfs) != 0) {
            		ret = -1;
            		goto END;
            }
            LOG("11", "12", "[INFO] -----------filename_id_fastDFS= %s", file_id_fdfs);
        /*************************将文件的信息 写入到redis数据库中**********************************/
        
        /*************************1、拼接URL连接，使前端根据该路径可以直接访问对应的资源文件**********************************/
        		if (file_url_make(file_id_fdfs, file_url_fdfs)!= 0) {
        				ret = -1;
        				goto END;
        		}
        /*************************2、拼接URL连接，使前端根据该路径可以直接访问对应的资源文件**********************************/
						if (file_msg_store_to_redis(file_id_fdfs, file_url_fdfs, file_name, /*user*/"zhao") != 0) {
								ret = -1;
								goto END;
						}
						/*************************删除本地存储的 上传文件**********************************/
						unlink(file_name);
						
				memset(file_name, 0, sizeof(file_name));
        memset(file_id_fdfs, 0, sizeof(file_id_fdfs));
        memset(file_url_fdfs, 0, sizeof(file_url_fdfs));
        memset(text_source, 0, sizeof(text_source));


    } /* while */
END:
    if (text_source != NULL) {
        free(text_source);	
    }
    return ret;
}








