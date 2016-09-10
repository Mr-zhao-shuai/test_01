
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "echo_op.h"
#include "make_log.h"
#include "redis_op.h"
#include "redis_keys.h"

#define FILE_NAME_LOG_ECHO_OP 								"11"    //定义Log文件夹的名字
#define PRE_DES_LOG_ECHO_OP   								"12"    //定义Log文件的前缀描述


#define MAX_BOUNDARY_BUF 											(512)
//#define MAX_FILE_ID_FDFS											(256)   //定义返回 fastFDS的字符串的长度

/*获取文件的后缀  
 *@file_name 			  文件的名字
 *@suffix 					传出参数，文件的后缀    需要在函数外部申请空间
 *				
 *return value      正确返回 0  失败返回 非0
 *
*/
int file_suffix_get(char *file_name, char *suffix)
{
//		int 					ret = 0;
//		int 					i = 0;
//		char 				  *tem_p = NULL;
//		
//		if (file_name == NULL || suffix == NULL) {
//		    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROER] argument error");
//		    ret = -1;
//		    goto END;
//		}
//		LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] file_name[%s]", file_name);
//		tem_p = strstr(file_name, ".");
//		if (tem_p == NULL) {
//				strcpy(suffix, "null");
//				goto END;
//		} else {
//			  tem_p++;
//				while (tem_p != '\0') {
//				    suffix[i] = *tem_p;
//				    i++;
//				    tem_p++;
//				}
//				suffix[++i] = '\0';
//		}
//		
//		LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] file suffix is [%s]", suffix);
//		
//END:
//		return ret;		
    const char *p = file_name;
    int len = 0;
    const char *q=NULL;
    const char *k= NULL;

    if (p == NULL) {
        return -1;
    }

    q = p;

    //asdsd.doc.png
    //             ↑

    while (*q != '\0') {
        q++;
    }

    k = q;
    while (*k != '.' && k != p) {
        k--;
    }

    if (*k == '.') {
        k++;
        len = q - k;

        if (len != 0) {
            strncpy(suffix, k, len);
            suffix[len] = '\0';
        }
        else {
            strncpy(suffix, "null", 5);
        }
    }
    else {
        strncpy(suffix, "null", 5);
    }

    return 0;
}







/*获取网络上传的文件信息，并保存至本地，用相同的名字命名
 *@source_net 			获取的网页信息
 *@len 						  上传后得到的文件的名字    需要在函数外部申请空间
 *@file_name				上传的文件的名字          需要在函数外部申请空间
 *return value      正确返回 0  失败返回 非0
 *
*/
int fiel_source_get(char *source_net, int len, char *file_name)
{
    int 				ret = 0;											//返回值

    int 				i = 0;												//临时变量
    int         is_Linux_flag = 0;                                  //

    char 				boundary[MAX_BOUNDARY_BUF] = {0};
    char 				*tem_source = NULL;      			                //辅助指针
    char  		  *tem_char_p = NULL;
    int 				fd = 0;												//创建的文件

    if (source_net == NULL || file_name == NULL) {
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] argument error");
        ret = -1;
        goto END;
    }

    tem_source = source_net;

//    for (i = 0; i < len; i++) {
//        if ((ch = getchar()) < 0) {
//             LOG("11", "12", "[INFO] Not enough bytes received on standard input ");
//             ret = -1;
//             goto END;
//            LOG("11", "12", "[INFO] Not enough bytes received on standard input ");
//            break;
//        }
//        source_net[i] = ch;
//    }

    //查找上传的文件名
    tem_source = strstr(source_net, "filename=");
    if (tem_source == NULL) {
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] Not found filename=");
        ret = -1;
        goto END;
    }

    tem_source += strlen("filename=");
    tem_source = strchr(tem_source, '"');
    tem_source++;
    i = 0;
    while (*(tem_source) != '"') {   
        file_name[i] = *tem_source;
        i++;
        tem_source++;
    }
    file_name[++i] = '\0';
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] filename=%s", file_name);


    //找出文件内容    区分win平台和Linux平台 
    tem_source = strstr(source_net, "\r\n");				//判断是否为Win平台
    if (tem_source == NULL) {
        tem_source = strstr(source_net, "\n");			//判断是否为Linux平台
        if (tem_source == NULL) {
            LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] Not found boundary!");
            ret = 1;        
            goto END;
        }
    } 
    if (tem_source - source_net > MAX_BOUNDARY_BUF) {
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] boundary too long! or find \r\n in Linux");
    }
    strncpy(boundary, source_net, tem_source - source_net);
    boundary[tem_source - source_net] = '\0'; 

    tem_char_p = tem_source;
    tem_source = strstr(tem_char_p, "\r\n\r\n");			//判断是否为Win平台
    if (tem_source == NULL) {
        tem_source = strstr(tem_char_p, "\n\n");			//判断是否为Linux平台
        if (tem_source == NULL) {
            LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] Not the begin of net source!");
            ret = 1;        
            goto END;
        }
        tem_source += 2;
        is_Linux_flag = 1;
    } else {
        tem_source += 4;
    } 

    tem_char_p = memstr(tem_source, len - (tem_source - source_net), boundary);
    if (tem_char_p == NULL) {
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] Not find net source!");
        ret = -1;
        goto END;
    }


    fd = open(file_name, O_CREAT|O_WRONLY, 0644);
    if (fd < 0) {
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] open %s error", file_name);
        ret = -1;
        goto END;
    }

    if (is_Linux_flag) {
        ftruncate(fd, (tem_char_p - tem_source - 1));
        write(fd, tem_source, (tem_char_p - tem_source - 1));
    } else {
        ftruncate(fd, (tem_char_p - tem_source - 2));
        write(fd, tem_source, (tem_char_p - tem_source - 2));
    }

    close(fd);

    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] upload [%s] succ!", file_name);      

END:
    memset(boundary, 0, sizeof(boundary));

    return ret; 
}

/*将文件上传至fastDFS系统，并获得存储的文件名字
 *@file_name 				要上传的文件的名字
 *@file_name_fdfs   上传后得到的文件的名字    需要在函数外部申请空间
 *return value      正确返回 0  失败返回 非0
 *
*/
int recv_file_upload(const char *file_name, char *file_id_fdfs)
{
		int 					ret = 0;
		
    pid_t 				pid = 0;
    int 					fd_pipe[2];
    
    if (file_name == NULL || file_id_fdfs == NULL) {
    		LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] argument of tecv_file_upload error");
    }

    if (pipe(fd_pipe) < 0) {
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] pipe error");
        ret = -1;
        goto END;
    }

    pid = fork();
    if (pid == 0) {
        //chlid   
        close(fd_pipe[0]);			//关闭读端
        
        dup2(fd_pipe[1], STDOUT_FILENO);          //将标准输出 重定向到管道中,使  执行系统命令 时的输出输出至管道中
        execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", file_name, NULL);
       
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] exec fdfs_upload_file error");
    } else {
        //parent
        close(fd_pipe[1]);			//关闭写端
        wait(NULL);
        
        read(fd_pipe[0], file_id_fdfs, MAX_FILE_ID_FDFS);			//从管道中去读数据
				file_id_fdfs[strlen(file_id_fdfs) - 1] = '\0';
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] file_id_fdfs = [%s]", file_id_fdfs);
        
    }
END:
	return ret;
}


/*获得fastDFS中，文件的存储的路径
 *@file_id_fdfs 		fastDFS中，文件的存储名字
 *@file_url_fdfs    拼接好后的URL路径      需要在函数外部申请空间
 *return value      正确返回 0  失败返回 非0
 *
*/
int file_url_make(char *file_id_fdfs, char *file_url_fdfs)
{
		int 			ret = 0;

    char 			*tem_p = NULL;
    char 			*tem_q = NULL;

    char 			fdfs_file_stat_buf[512] = {0};     //文件系统查询的信息
    char 			fdfs_file_host_name[128] = {0};    //文件存储的ip地址
    //cahr 			tem_buf_file_id[256] = [0];

    int 			fd_pipe[2];
    pid_t 		pid;

    pipe(fd_pipe);
    //strcpy(tem_buf_file_buf, file_id_fdfs);
    

    pid = fork();
    if (pid == 0) {
        //child
        //关闭读管道
        close(fd_pipe[0]);

        //将标准输出重定向到写管道
        dup2(fd_pipe[1], STDOUT_FILENO);
        execlp("fdfs_file_info", "fdfs_file_info", "/etc/fdfs/client.conf", file_id_fdfs, NULL);
        close(fd_pipe[1]);
    }

    //关闭写管道
    close(fd_pipe[1]);
    //read from 读管道
    read(fd_pipe[0], fdfs_file_stat_buf, 512);
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] get file_ip_fdfs [%s] succ\n", fdfs_file_stat_buf);
    wait(NULL);
    close(fd_pipe[0]);

    //拼接上传文件的完整url地址--->http://host_name/group1/M00/00/00/D12313123232312.png
    tem_p = strstr(fdfs_file_stat_buf, "source ip address: ");

    //192.168.123.123
    //q              k
    tem_p = tem_p + strlen("source ip address: ");
    tem_q = strstr(tem_p, "\n");
    strncpy(fdfs_file_host_name, tem_p, tem_q - tem_p);

    fdfs_file_host_name[tem_q - tem_p] = '\0';

    strcat(file_url_fdfs, "http://");
    strcat(file_url_fdfs, fdfs_file_host_name);
    strcat(file_url_fdfs, "/");
    strcat(file_url_fdfs, file_id_fdfs);

    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] get file_url_fdfs [%s] succ\n", file_url_fdfs);

    return ret;
}



int file_msg_store_to_redis(char *file_id_fdfs, char *file_url_fdfs, char *file_name, char *user)
{
		int 					ret = 0;
    redisContext 	*redis_conn = NULL;       	//redis 连接句柄

    char 					*redis_value_buf = NULL;  	//拼接好的字符串
    time_t 				now;
    char 					create_time[128]; 					//创建时间
    char 					suffix[64];       					//文件后缀
    //char 					*cli_ip = "127.0.0.1";
    //char 					*port = "6379";

    //redis_conn = rop_connectdb_nopwd(cli_ip, port);
    //redis_conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO]  connect redis start");
    redis_conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (redis_conn == NULL) {
    	  LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROR] not connect redis");
        ret= -1;
        goto END;
    }
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] connect redis succ");

    //make file value
    redis_value_buf = malloc(1024);
    if (redis_value_buf == NULL) {
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[ERROER] malloc redis value error\n");
        ret = -1;
        goto END;
    }

    memset(redis_value_buf, 0, 1024);

    //fileid
    strcat(redis_value_buf, file_id_fdfs);
    strcat(redis_value_buf, "|");
    //url
    strcat(redis_value_buf, file_url_fdfs);
    strcat(redis_value_buf, "|");
    //filename
    strcat(redis_value_buf, file_name);
    strcat(redis_value_buf, "|");
    //create time
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] store file name succ");
    now = time(NULL);
    strftime(create_time, 128-1, "%Y-%m-%d %H:%M:%S", localtime(&now));
    strcat(redis_value_buf, create_time);
    strcat(redis_value_buf, "|");
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] store time succ");
    //user
    strcat(redis_value_buf, user);
    strcat(redis_value_buf, "|");
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] store user succ");
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] [%s]", redis_value_buf);
    //type
    if ( file_suffix_get(file_name, suffix) == 0 ) {
        strcat(redis_value_buf, suffix);
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] store suffix succ");
        LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] [%s]", redis_value_buf);
    }
    

    //将文件插入到FILE_INFO_LIST表中
    rop_list_push(redis_conn, "FILE_INFO_LIST", redis_value_buf);

    //将文件插入到FILE_HOT_ZSET中，默认的权值是1
    rop_zset_increment(redis_conn, "FILE_HOT_ZSET", file_id_fdfs);

    free(redis_value_buf);
    LOG(FILE_NAME_LOG_ECHO_OP, PRE_DES_LOG_ECHO_OP, "[INFO] store redis suss");
END:
	  if (redis_conn) {
	      rop_disconnect(redis_conn);
	  }
    
    return ret;
}






//find 'substr' from a fixed-length buffer   
//('full_data' will be treated as binary data buffer)  
//return NULL if not found        
/*在二进制内存中查找对应的字符串
 *@full_data    		输入字符串
 *@full_data_len   	输入字符串的长度
 *@substr        		要找的子串
 *  return value    成功  返回找到的子串地址
 *                  失败  返回NULL
 */       
char* memstr(char* full_data, int full_data_len, char* substr)  
{  
    int 				i = 0;  
    char 				*cur = NULL;  
    int 				last_possible = 0; 
    int 				sublen = 0; 

    if (full_data == NULL || full_data_len <= 0 || substr == NULL) {  
        return NULL;  
    }  

    if (*substr == '\0') {  
        return NULL;  
    }  

    sublen = strlen(substr);  
    cur = full_data;  
    last_possible = full_data_len - sublen + 1;  

    for (i = 0; i < last_possible; i++) {  
        if (*cur == *substr) {  
            //assert(full_data_len - i >= sublen);  
            if (memcmp(cur, substr, sublen) == 0) {  
                //found  
                return cur;  
            }  
        }  
        cur++;  
    }  

    return NULL;  
}     
