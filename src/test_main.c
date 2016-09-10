#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "make_log.h"
#include "redis_op.h"
#include "redis_keys.h"

void func(int a, int b)
{
    printf("a = %d, b = %d\n", a, b);
}

int main(int argc, char *argv[])
{
    int ret = 0;

    char *cli_ip = "127.0.0.1";
    char *port = "6379";
    char tem_buf[128] = {0};

    redisContext * hdl_rd_conn = NULL;

    hdl_rd_conn = rop_connectdb_nopwd(cli_ip, port);
    if (hdl_rd_conn == NULL) {
        printf("not connect\n");
        ret = -1;
        goto END;
    }
    printf("connect succ\n");

    ret = rop_get_string(hdl_rd_conn, "name", tem_buf);
    if (ret != 0) {
        printf("rop_get_string error\n");
        ret = -1;
        goto END;
    }
    tem_buf[7] = '\0';
    printf("%s\n", tem_buf);


    rop_disconnect(hdl_rd_conn);
    

END:
	return ret;
}
