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

/** const 指针形式,彩色 terminal  */
char *GREEN    = "\e[1;32m";
char *BLUE     = "\e[1;34m";
char *YELLOW   = "\e[1;33m";
char *RED      = "\e[1;31m";
char *MAGENTA  = "\e[01;35m";
char *CYAN     = "\e[01;36m";
char *NORMAL   = "\e[0m";

#define logprintf(format, arg...) fprintf(stderr, "%s[LOG]%s %s:%d:%s "format"\n",\
        RED, NORMAL, __FILE__, __LINE__, __func__, ##arg)
#define printfln(format, arg...) fprintf(stderr, format"\n", ##arg)

int main(int argc, char *argv[])
{

    return 0;
}

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */

