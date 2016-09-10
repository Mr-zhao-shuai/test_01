#ifndef _DATA_OP_H__
#define _DATA_OP_H__


int query_string_value_get(const char *query, const char *key, char *value, int *value_len);
char *file_list_to_json_print(int fromId, int count, char *cmd, char *kind);





#endif