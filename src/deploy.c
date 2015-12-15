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

#define DEBUG           0

#define BUF_LEN         128
#define PATH_BUF_LEN    1024
#define CMD_BUF_LEN     1024
#define LINE_BUF_LEN    1024
#define OUTPUT_BUF_LEN  1024 * 500

#if (DEBUG)
#define logprintf(format, arg...) fprintf(stderr, "%s[LOG]%s [%s:%d:%s] "format"\n",\
        RED, NORMAL, __FILE__, __LINE__, __func__, ##arg)
#else
#define logprintf(format, arg...) do{}while(0)
#endif

#define printfln(format, arg...) fprintf(stderr, format"\n", ##arg)
#define print_error(format, arg...) fprintf(stderr, "%s"format"%s\n", RED, ##arg, NORMAL)

static const char *VERSION  =  "v1.0.0";
#if defined(__DATE__) && defined(__TIME__)
static const char *BUILD_DATE = __DATE__ " " __TIME__;
#else
static const char *BUILD_DATE = "unknown";
#endif

typedef struct _g_cfg_t
{
    char project[BUF_LEN];
    char opt[BUF_LEN];
    char refs_head[PATH_BUF_LEN];

    char user[BUF_LEN];
    char path[BUF_LEN];

    int    host_count;
    char **hosts_conf;
} g_cfg_t;

typedef struct _tid_cntr_t
{
    int tid;
} tid_cntr_t;

/** 线程工作空间 */
typedef struct _thread_data_t
{
    char *line_buf;
    char *cmd_buf;
    char *output_buf;
} thread_data_t;

/** 全局状态码/错误码 */
typedef enum _g_error_code_e
{
    CAN_NOT_OPEN_FILE       = 111,
    MALLOC_JSON_PARSER_FAIL,
    PARSE_JSON_FAIL,
    PARSE_CONFIG_EXCEPTION,
    MALLOC_THREAD_ARGS_FAIL,
    MALLOC_THREAD_DATA_FAIL,
    CREATE_THREAD_FAIL,
    INIT_THREAD_FAIL,
    POPEN_FAIL,
} g_error_code_e;

/** const 指针形式,彩色 terminal  */
char *GREEN    = "\e[1;32m";
char *BLUE     = "\e[1;34m";
char *YELLOW   = "\e[1;33m";
char *RED      = "\e[1;31m";
char *BLINK_RED= "\e[1;31;5m";
char *MAGENTA  = "\e[1;35m";
char *CYAN     = "\e[1;36m";
char *NORMAL   = "\e[0m";
char *EMPTY    = "";

/** 全局变量 */
g_cfg_t g_cfg;
pthread_mutex_t work_mutex;
thread_data_t  *thread_data;

void
usage(const char *argv_0)
{
    printfln("\n%s-----USAGE----%s", CYAN, NORMAL);
    printfln("%s%s %sproject %sdeploy             %sdeploy project with latest <head>.%s", GREEN, argv_0, BLUE, YELLOW, RED, NORMAL);
    printfln("%s%s %sproject %srollback %s<head>    %srollback with <head>.%s", GREEN, argv_0, BLUE, YELLOW, MAGENTA, RED, NORMAL);
    printfln("version %s, build at %s\n", VERSION, BUILD_DATE);

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

        snprintf(g_cfg.refs_head, PATH_BUF_LEN, "%s", argv[3]);
        logprintf("g_cfg.refs_head = %s", g_cfg.refs_head);
    }

    logprintf("g_cfg.project = %s", g_cfg.project);
    logprintf("g_cfg.opt = %s", g_cfg.opt);

    /** 解析配置文件 */
    char cfg_file[PATH_BUF_LEN];
    snprintf(cfg_file, PATH_BUF_LEN, "conf/%s.json", g_cfg.project);
    logprintf("cfg_file: %s", cfg_file);

    // 从文件中读取要解析的JSON数据
    FILE *fp = fopen(cfg_file, "r");
    if (NULL == fp)
    {
        print_error("Can NOT open config file: %s", cfg_file);
        exit(CAN_NOT_OPEN_FILE);
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *data = (char*)malloc(len + 1);
    if (NULL == data)
    {
        print_error("Can NOT malloc for json paser.");

        fclose(fp);
        exit(MALLOC_JSON_PARSER_FAIL);
    }

    fread(data, 1, len, fp);
    fclose(fp);

    // 解析JSON数据
    cJSON *root_json = cJSON_Parse(data);    // 将字符串解析成json结构体
    if (NULL == root_json)
    {
        print_error("parse JSON error: %s", cJSON_GetErrorPtr());

        cJSON_Delete(root_json);
        free(data);
        exit(PARSE_JSON_FAIL);
    }

    cJSON *user = cJSON_GetObjectItem(root_json, "user");
    if (NULL == user)
    {
        print_error("Lost config: user");
        goto PARSE_EXCEPTION;
    }
    snprintf(g_cfg.user, BUF_LEN, "%s", user->valuestring);
    logprintf("g_cfg.user: %s", g_cfg.user);

    cJSON *path = cJSON_GetObjectItem(root_json, "path");
    if (NULL == path)
    {
        print_error("Lost config: path");
        goto PARSE_EXCEPTION;
    }
    snprintf(g_cfg.path, BUF_LEN, "%s", path->valuestring);
    logprintf("g_cfg.path: %s", g_cfg.path);

    cJSON *hosts_conf = cJSON_GetObjectItem(root_json, "hosts_conf");
    if (NULL == hosts_conf)
    {
        print_error("Lost config: hosts_conf");
        goto PARSE_EXCEPTION;
    }
    g_cfg.host_count = cJSON_GetArraySize(hosts_conf);
    logprintf("g_cfg.host_count = %d", g_cfg.host_count);
    if (! (g_cfg.host_count > 0 ))
    {
        print_error("Need config host.");
        goto PARSE_EXCEPTION;
    }
    g_cfg.hosts_conf = (char **)malloc(sizeof(char *) * g_cfg.host_count);
    if (NULL == g_cfg.hosts_conf)
    {
        print_error("malloc hosts_conf fail.");
        goto PARSE_EXCEPTION;
    }
    int i = 0;
    for (; i < g_cfg.host_count; i++)
    {
        g_cfg.hosts_conf[i] = (char *)malloc(sizeof(char) * BUF_LEN);
        if (NULL == g_cfg.hosts_conf[i])
        {
            print_error("malloc for host buf fail. id: %d", i);
            free(g_cfg.hosts_conf);
            goto PARSE_EXCEPTION;
        }

        cJSON *item = cJSON_GetArrayItem(hosts_conf, i);
        snprintf(g_cfg.hosts_conf[i], BUF_LEN, "%s", item->valuestring);
        logprintf("g_cfg.hosts_conf[%d] = %s", i, g_cfg.hosts_conf[i]);
    }

    cJSON_Delete(root_json);
    free(data);

    return 0;

PARSE_EXCEPTION:
    cJSON_Delete(root_json);
    free(data);
    exit(PARSE_CONFIG_EXCEPTION);
}

void check_tty()
{
    if (! isatty(fileno(stdout)))
    {
        GREEN    = EMPTY;
        BLUE     = EMPTY;
        YELLOW   = EMPTY;
        RED      = EMPTY;
        BLINK_RED= EMPTY;
        MAGENTA  = EMPTY;
        CYAN     = EMPTY;
        NORMAL   = EMPTY;
    }
}

void
exec_cmd(int tid)
{
    logprintf("tid: %d", tid);

    thread_data_t *t_data = &thread_data[tid];

    snprintf(t_data->line_buf, LINE_BUF_LEN,
            "######## %sSTART %s%s %s%s@%s%s ########\n",
            CYAN, MAGENTA, g_cfg.opt,
            YELLOW, g_cfg.user, g_cfg.hosts_conf[tid], NORMAL);
    strncat(t_data->output_buf, t_data->line_buf, LINE_BUF_LEN);

    /** 执行命令 */
    int   rc = 0;
    FILE *fp;

    fp = popen(t_data->cmd_buf, "r");
    if (NULL == fp) {
        snprintf(t_data->line_buf, LINE_BUF_LEN, "%sCan NOT popen(%s).%s\n", RED, t_data->cmd_buf, NORMAL);
        goto FINISH;
    }

    while (strlen(t_data->output_buf) < OUTPUT_BUF_LEN - LINE_BUF_LEN && NULL != fgets(t_data->line_buf, LINE_BUF_LEN, fp))
    {
        strncat(t_data->output_buf, t_data->line_buf, strlen(t_data->line_buf));
    }

    rc = pclose(fp);
    int status_child = WEXITSTATUS(rc);

    if (EXIT_SUCCESS == status_child)
    {
        snprintf(t_data->line_buf, LINE_BUF_LEN, "%s%s%s SUCCESS %s@%s%s\n",
                MAGENTA, g_cfg.opt, GREEN, g_cfg.user, g_cfg.hosts_conf[tid], NORMAL);
    }
    else
    {
        snprintf(t_data->line_buf, LINE_BUF_LEN, "%s%s%s FAIL %s@%s%s\n",
                MAGENTA, g_cfg.opt, BLINK_RED, g_cfg.user, g_cfg.hosts_conf[tid], NORMAL);
    }
    if (strlen(t_data->output_buf) < OUTPUT_BUF_LEN - LINE_BUF_LEN)
    {
        strncat(t_data->output_buf, t_data->line_buf, LINE_BUF_LEN);
    }

FINISH:
    snprintf(t_data->line_buf, LINE_BUF_LEN, "######## %sEND%s ########\n\n", CYAN, NORMAL);
    if (strlen(t_data->output_buf) < OUTPUT_BUF_LEN - LINE_BUF_LEN)
    {
        strncat(t_data->output_buf, t_data->line_buf, LINE_BUF_LEN);
    }

    return;
}

void
output_result(int tid)
{
    thread_data_t *t_data = &thread_data[tid];

    pthread_mutex_lock(&work_mutex);
    fprintf(stderr, "%s", t_data->output_buf);
    pthread_mutex_unlock(&work_mutex);

    return;
}

void *
deploy_worker(void *arg)
{
    tid_cntr_t *p_tid_cntr = (tid_cntr_t *)arg;
    int tid = p_tid_cntr->tid;
    logprintf("tid = %d", tid);

    thread_data_t *t_data = &thread_data[tid];
    snprintf(t_data->cmd_buf, CMD_BUF_LEN,
            "ssh %s@%s \"cd %s && git pull 2>&1 && git log -1 | awk '{if (\\$1 ~/commit/) {print \\$2}}'\" 2>&1",
            g_cfg.user, g_cfg.hosts_conf[tid], g_cfg.path);
    logprintf("tid: %d, cmd: %s", tid, t_data->cmd_buf);

    exec_cmd(tid);
    output_result(tid);

    return NULL;
}

void *
rollback_worker(void *arg)
{
    tid_cntr_t *p_tid_cntr = (tid_cntr_t *)arg;
    int tid = p_tid_cntr->tid;
    logprintf("tid = %d", tid);

    thread_data_t *t_data = &thread_data[tid];
    snprintf(t_data->cmd_buf, CMD_BUF_LEN,
            "ssh %s@%s \"cd %s && git reset --hard %s 2>&1 && git log -1 | awk '{if (\\$1 ~/commit/) {print \\$2}}'\" 2>&1",
            g_cfg.user, g_cfg.hosts_conf[tid], g_cfg.path, g_cfg.refs_head);
    logprintf("tid: %d, cmd: %s", tid, t_data->cmd_buf);

    exec_cmd(tid);
    output_result(tid);

    return NULL;
}

void
handle_work(int opt_num)
{
    logprintf("start handle_work ...");

    fprintf(stderr, "\n        ___---=== %s%s%s ===---____        \n\n", BLUE, g_cfg.project, NORMAL);

    pthread_t *pt_dw_core;
    int i;
    tid_cntr_t *tid_cntr;

    tid_cntr = (tid_cntr_t *)malloc(sizeof(tid_cntr_t) * g_cfg.host_count);
    if (NULL == tid_cntr)
    {
        print_error("malloc thread data fail, exit.");
        exit(MALLOC_THREAD_ARGS_FAIL);
    }

    pt_dw_core = (pthread_t *)malloc(sizeof(pthread_t) * g_cfg.host_count);
    if (! pt_dw_core)
    {
        print_error("malloc threads ids for work thrads fail, exit.");
        exit(MALLOC_THREAD_DATA_FAIL);
    }

    for (i = 0; i < g_cfg.host_count; ++i)
    {
        tid_cntr[i].tid = i;

        int ret;
        if (1 == opt_num)
        {
            ret = pthread_create(&pt_dw_core[i], NULL, deploy_worker, (void *)&tid_cntr[i]);
        }
        else
        {
            ret = pthread_create(&pt_dw_core[i], NULL, rollback_worker, (void *)&tid_cntr[i]);
        }

        if (0 != ret)
        {
            print_error("create the %dth pt_dw_core thread fail, exit.", i);
            exit(CREATE_THREAD_FAIL);
        }
    }

    for (i = 0; i < g_cfg.host_count; i++)
    {
        pthread_join(pt_dw_core[i], NULL);
    }
}

void init_thread()
{
    thread_data = (thread_data_t *)malloc(sizeof(thread_data_t) * g_cfg.host_count);
    if (NULL == thread_data)
    {
        print_error("malloc for thread_data fail.");
        goto INIT_THREAD_EXCEPTION;
    }

    int i = 0;
    for (; i < g_cfg.host_count; i++)
    {
        thread_data[i].line_buf   = (char *)malloc(sizeof(char) * LINE_BUF_LEN);
        thread_data[i].cmd_buf    = (char *)malloc(sizeof(char) * CMD_BUF_LEN);
        thread_data[i].output_buf = (char *)malloc(sizeof(char) * OUTPUT_BUF_LEN);
        if (NULL == thread_data[i].line_buf ||NULL == thread_data[i].cmd_buf || NULL == thread_data[i].output_buf)
        {
            print_error("malloc for thread_data[%d]->[member] fail.", i);
            free(thread_data);
            goto INIT_THREAD_EXCEPTION;
        }

        thread_data[i].line_buf[0]   = '\0';
        thread_data[i].cmd_buf[0]    = '\0';
        thread_data[i].output_buf[0] = '\0';
    }

    return;

INIT_THREAD_EXCEPTION:
    exit(INIT_THREAD_FAIL);
}

void do_work(const char *argv_0)
{
    init_thread();

    if (0 == strcmp("deploy", g_cfg.opt))
    {
        // 部署 deploy
        handle_work(1);
    }
    else if (0 == strcmp("rollback", g_cfg.opt))
    {
        // 回滚 rollback
        handle_work(2);
    }
    else
    {
        usage(argv_0);
    }

    fprintf(stderr, "%sWork done.%s\n\n", GREEN, NORMAL);
}

int main(int argc, char *argv[])
{
    check_tty();
    parse_arg(argc, argv);
    do_work(argv[0]);

    return 0;
}

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */

