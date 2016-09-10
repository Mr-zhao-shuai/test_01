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
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"


static void PrintEnv(char *label, char **envp)
{
    printf("%s:<br>\n<pre>\n", label);
    for ( ; *envp != NULL; envp++) {
        printf("%s\n", *envp);
    }
    printf("</pre><p>\n");
}

int main ()
{
    char **initialEnv = environ;
    int count = 0;
    int j = 0, flag = 0, doit = 0;
    int j2 = 0, flag2 = 0, doit2 = 0;
    FILE* fp;
    int fd;
    char buf[5] = {'\r', '\n', '\r', '\n', '\0'};
    char tmp[5] = {0};
    char buf2[128] = {'\n', '-','-','-','-','-','-', 0};
    char tmp2[128] = {0};
    char *p = NULL;
    int len2 = 0;
    //fd = open("./a.txt", O_CREAT|O_WRONLY, 0644);
    fp = fopen("./a.txt", "w+");
    //fp = fdopen(fd, "w+");

    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;


        printf("Content-type: text/html\r\n"
                "\r\n"
                "<title>FastCGI echo</title>"
                "<h1>FastCGI echo</h1>\n"
                "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
            //ftruncate(fileno(fp), len);
            printf("len %d\n", len);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
            printf("No data from standard input.<p>\n");
        }
        else {
            int i, ch;

            PrintEnv("Request environment", environ);
            PrintEnv("Initial environment", initialEnv);

            printf("SunYan input:<br>\n<pre>\n");
            printf("<i>%s</i>\n", getenv("CONTENT_TYPE"));

            p = strstr(getenv("CONTENT_TYPE"), "WebKitFormBoundary");
            strcat(buf2, p);
            len2 = strlen(buf2);
            printf("len2 %d\n", len2);

            printf("<i>%s</i>\n", p);
            printf("<i>%s</i>\n", buf2);

            printf("Standard input:<br>\n<pre>\n");
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
#if 1
                if(doit == 1) {
                    //putchar(ch);
                    //fwrite(&ch, 1, 1, fp);
#if 1
                    if(doit2 == 0) {
                        //putchar(ch);
                        // fwrite(&ch, 1, 1, fp);
                        if (flag2 == 1) {
                            tmp2[j2] = ch;
                            if (buf2[j2] == tmp2[j2]) {
                                j2++;
                                if (len2 == j2) {
                                    doit2 = 1;
                                    len2 = ftell(fp);
                                    //ftruncate(fileno(fp), len2 - 1);
                                }
                            } else {
                                j2++;
                                flag2 = 0;
                                fwrite(tmp2, 1, j2, fp);
                                printf("i = %d\t value = %s\n", i, tmp2);
                                memset(tmp2, 0, 128);
                                j2 = 0;
                            }
                        } else {

                            if (ch == '\n') {
                                j2 = 0;
                                tmp2[j2++] = ch;
                                flag2 = 1;
                            } else {
                                fwrite(&ch, 1, 1, fp);
                                printf("i = %d\t value = %d\n", i, &ch);
                                flag2 = 0;
                            }

                        }

                    }

#endif
                } else {
                    if (flag == 1) {
                        tmp[j++] = ch;
                        if (j == 4) {
                            if (!strcmp(buf, tmp)) {
                                doit = 1;
                            } else {
                                flag = 0;
                            }
                        }
                    }

                    if (ch == '\r' && flag == 0) {
                        j = 0;
                        tmp[j++] = ch;
                        flag = 1;
                    }

                }

#endif
#if 0
                putchar(ch);
                fwrite(&ch, 1, 1, fp);
#endif

            }
            printf("\n</pre><p>\n");
            printf("len2 2 %d\n", len2);
            printf("i %d\n", i);
        }

    } /* while */

    fclose(fp);

    return 0;
}
