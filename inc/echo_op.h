#ifndef _ECHO_OP_H__
#define _ECHO_OP_H__

#define MAX_FILE_ID_FDFS											(256)   //定义返回 fastFDS的字符串的长度

char* memstr(char* full_data, int full_data_len, char* substr);
int fiel_source_get(char *source_net, int len, char *file_name);
int recv_file_upload(const char *file_name, char *file_id_fdfs);
int file_url_make(char *file_id_fdfs, char *file_url_fdfs);

int file_msg_store_to_redis(char *file_id_fdfs, char *file_url_fdfs, char *file_name, char *user);
int file_suffix_get(char *file_name, char *suffix);








#endif
