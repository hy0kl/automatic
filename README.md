# git 利用 web hook 实现自动部署

# automatic
git&amp;gitlab web-hook 自动部署项目脚本

# 文件简要说明

```
.
├── conf        # 部署项目配置文件
├── script      # shell + php 部署脚本
└── src         # c 语言写的部署工具
```

# 使用说明
## 简介
当待部署的机器比较多时,用 shell 来串行部署会有明显的部署延后现象.

建议使用c语言版的并行部署工具.

##  使用方法

```
1. 在本项目主目录下执行 make,会生成 deploy 工具.
2. 直接执行 deploy 工具,会提示使用说明:
$ ./deploy

-----USAGE----
./deploy project deploy             deploy project with latest <head>.
./deploy project rollback <head>    rollback with <head>.
version v1.0.0, build at Oct 23 2015 17:44:16

3. 在 conf/ 下参考 demo.json 来写部署的配置文件.
4. 以 demo 项目为例,批量并行部署上线代码:
$ ./deploy demo deploy
```

# 问题
实际部署的时候,发现随机报出以下错: `ssh_exchange_identification: Connection closed by remote host
fatal: The remote end hung up unexpectedly`

经过多方定位,发现是 gitlab 服务器 sshd\_config MaxStartups 设置过小,导致并行 pull 代码时因为链接数不够而被拒了.

前面定位问题时走了弯路,一直在发布机上找原因.

解决方法:

```
到 gitlab 机器上,将 sshd_config 中的 MaxStartups 改为一个较大的数,重启 sshd.
```
