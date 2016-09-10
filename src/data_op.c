#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "data_op.h"
#include "make_log.h"
#include "redis_op.h"
#include "redis_keys.h"
#include "cJSON.h"
#include "util_cgi.h"



#define FILE_NAME_LOG_DATA_OP 								"data_op"    //定义Log文件夹的名字
#define PRE_DES_LOG_DATA_OP   								"data_op"    //定义Log文件的前缀描述

/*获取命令key值对应的value值  
 *@query_string 			  命令字符串
 *@key        					要查找的key值
 *@value                key对应的value         在函数外部申请空间
 *@value_len            如果忽略长度，可以传入NULL
 *				
 *return value          正确返回 0  没找到 返回1
 *
*/
int query_string_value_get(const char *query_string, const char *key, char *value, int *value_len)
{
		int  					ret = 0;
		
		char 					*key_found = NULL;
		char 					*tem_end = NULL;
		int 					tem_value_len = 0;
		
		if (query_string == NULL || key == NULL || value == NULL) {
					LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "[ERROR] argument error of query_string_value_get");
					ret = -1;
					goto END;
		}
		
		key_found = strstr(query_string, key);
		if (key_found == NULL) {
				LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "[INFO] not found in query string");
				ret = 1;
		    goto END;
		}
		
		key_found+=strlen(key);//=
    key_found++;//value

    //get value
    tem_end = key_found;

    while ('\0' != *tem_end && '#' != *tem_end && '&' != *tem_end ) {
        tem_end++;
    }

    tem_value_len = tem_end - key_found;

    strncpy(value, key_found, tem_value_len);
    value[tem_value_len] ='\0';

    if (value_len != NULL) {
        *value_len = tem_value_len;
    }
		
END:
		return ret;		
}






char *file_list_to_json_print(int fromId, int count, char *cmd, char *kind)
{
    int i = 0;

    char *return_value = NULL;
    
    cJSON *root = NULL; 
    cJSON *array =NULL;
    char *out;
    char filename[128] = {0};
    char create_time[64] ={0};
    char picurl[128] = {0};
    char suffix[32] = {0};
    char pic_name[64] = {0};
    char file_url[256] = {0};
    char file_id[128] = {0};
    char user[128] = {0};
    int retn = 0;
    int endId = fromId + count - 1;
    int score = 0;

    RVALUES file_list_values = NULL;
    int value_num;
    redisContext *redis_conn = NULL;

    redis_conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (redis_conn == NULL) {
        LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "redis connected error");
        return NULL;
    }

    LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "fromId:%d, count:%d",fromId, count);
    file_list_values = malloc(count*VALUES_ID_SIZE);
    if (file_list_values == NULL) {
        LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "[ERROR] malloc of file_list_values error");
    }

    retn = rop_range_list(redis_conn, "FILE_INFO_LIST", fromId, endId, file_list_values, &value_num);
    if (retn < 0) {
        LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "redis range list error");
        rop_disconnect(redis_conn);
        return NULL;
    }
    LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "value_num=%d\n", value_num);


    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    for (i = 0;i < value_num;i ++) {
        //array[i]:
        cJSON* item = cJSON_CreateObject();

        //id
        //cJSON_AddNumberToObject(item, "id", i);
        get_value_by_col(file_list_values[i], 1, file_id, VALUES_ID_SIZE-1, 0);
        cJSON_AddStringToObject(item, "id", file_id);

        //kind
        cJSON_AddNumberToObject(item, "kind", 2);

        //title_m(filename)
        get_value_by_col(file_list_values[i], 3, filename, VALUES_ID_SIZE-1, 0);
        LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "filename=%s\n", filename);

        cJSON_AddStringToObject(item, "title_m", filename);

        //title_s
        get_value_by_col(file_list_values[i], 5, user, VALUES_ID_SIZE-1, 0);
        cJSON_AddStringToObject(item, "title_s", user);

        //time
        get_value_by_col(file_list_values[i], 4, create_time, VALUES_ID_SIZE-1, 0);
        cJSON_AddStringToObject(item, "descrip", create_time);
        LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "create_time=%s\n", create_time);

        //picurl_m
        memset(picurl, 0, 128);
        strcat(picurl, "http://192.168.21.91");
        strcat(picurl, "/static/file_png/");


        get_file_suffix(filename, suffix);
        sprintf(pic_name, "%s.png", suffix);
        strcat(picurl, pic_name);
        cJSON_AddStringToObject(item, "picurl_m", picurl);

        //url
        get_value_by_col(file_list_values[i], 2, file_url, VALUES_ID_SIZE-1, 0);
        cJSON_AddStringToObject(item, "url", file_url);
        LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP, "file_url=%s", file_url);

        //pv
        score = rop_zset_get_score(redis_conn, FILE_HOT_ZSET, file_id);
        cJSON_AddNumberToObject(item, "pv", score-1);

        //hot 
        //cJSON_AddNumberToObject(item, "hot", i%2);
        cJSON_AddNumberToObject(item, "hot", 0);


        cJSON_AddItemToArray(array, item);

    }



    cJSON_AddItemToObject(root, "games", array);

    out = cJSON_Print(root);

    LOG(FILE_NAME_LOG_DATA_OP, PRE_DES_LOG_DATA_OP,"%s", out);
    
    return_value = calloc(1, 10240);
    strcpy(return_value, out);
    
    //printf("%s", out);

    free(file_list_values);
    free(out);

    rop_disconnect(redis_conn);
    
    return return_value;
}