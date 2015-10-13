/**
 * @describe: 采用c实现并发部署git项目
 * @author: Jerry Yang(hy0kle@gmail.com)
 * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "cJSON.h"

#define DEBUG           1
#define BUF_LEN         128
#define PATH_LEN        1024
#define CMD_BUF_LEN     1024
#define LINE_BUF_LEN    1024
#define OUTPUT_BUF_LEN  1024 * 500

typedef struct _g_cfg_t
{
    char project[BUF_LEN];
    char opt[BUF_LEN];
    char refs_head[PATH_LEN];

    char user[BUF_LEN];
    char path[BUF_LEN];

    cJSON *hosts_conf;
} g_cfg_t;

/** const 指针形式,彩色 terminal  */
char *GREEN    = "\e[1;32m";
char *BLUE     = "\e[1;34m";
char *YELLOW   = "\e[1;33m";
char *RED      = "\e[1;31m";
char *MAGENTA  = "\e[01;35m";
char *CYAN     = "\e[01;36m";
char *NORMAL   = "\e[0m";

#if (DEBUG)
#define logprintf(format, arg...) fprintf(stderr, "%s[LOG]%s %s:%d:%s "format"\n",\
        RED, NORMAL, __FILE__, __LINE__, __func__, ##arg)
#else
#define logprintf(format, arg...) do{}while(0)
#endif

#define printfln(format, arg...) fprintf(stderr, format"\n", ##arg)

/** 全局变量 */
g_cfg_t g_cfg;

void
usage(const char *argv_0)
{
    printfln("%s-----USAGE----%s", CYAN, NORMAL);
    printfln("%s%s %sproject %sdeploy             %sdeploy project with latest <head>.%s", GREEN, argv_0, BLUE, YELLOW, RED, NORMAL);
    printfln("%s%s %sproject %srollback %s<head>    %srollback with <head>.%s", GREEN, argv_0, BLUE, YELLOW, MAGENTA, RED, NORMAL);

    exit(EXIT_SUCCESS);
}

int
parse_arg(int argc, char *argv[])
{
    if (! (argc > 2))
    {
        usage(argv[0]);
    }

    snprintf(g_cfg.project, BUF_LEN, "%s", argv[1]);
    snprintf(g_cfg.opt, BUF_LEN, "%s", argv[2]);

    if (0 == strcmp("rollback", g_cfg.opt))
    {
        if (! (argc > 3))
        {
            usage(argv[0]);
        }

        snprintf(g_cfg.refs_head, PATH_LEN, "%s", argv[3]);
        logprintf("g_cfg.refs_head = %s", g_cfg.refs_head);
    }

    logprintf("g_cfg.project = %s", g_cfg.project);
    logprintf("g_cfg.opt = %s", g_cfg.opt);

    /** 解析配置文件 */
    char cfg_file[PATH_LEN];
    snprintf(cfg_file, PATH_LEN, "conf/%s.json", g_cfg.project);
    logprintf("cfg_file: %s", cfg_file);

    return 0;
}

int main(int argc, char *argv[])
{
    parse_arg(argc, argv);

    if (0 == strcmp("deploy", g_cfg.opt))
    {
        // 部署
    }
    else if (0 == strcmp("rollback", g_cfg.opt))
    {
        // 回滚
    }
    else
    {
        usage(argv[0]);
    }

    return 0;
}

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */

