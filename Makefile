# VPATH = src:../headers
# %: 匹配零或若干字符
# $< 表示所有的依赖目标集
# $@ 表示目标集
# 用 @ 字符在命令行前,此命令将不被 make 显示出来
# 在 Makefile 的命令行前加一个减号 - (在 Tab 键之后),标记为不管命令出不出错都认为是成功的

LIBDIR = \
		-lpthread \
		-lm

GCC = gcc
GCCFLAGS = -g -Wall -Winline -pipe

TARGET = deploy

OBJS  = src/cJSON.o src/deploy.o

all : $(TARGET)
	@echo "Compile project"
	rm -f src/*.o

$(TARGET) : $(OBJS)
	@echo "Start compiler"
	$(GCC) -g -o $@ $^ $(LIBDIR)

%.o : %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS)

