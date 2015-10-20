# git 利用 web hook 实现自动部署

# automatic
git&amp;gitlab web-hook 自动部署项目脚本

# 文件简要说明

```
.
├── conf        # 部署项目配置文件
├── script      # shell + php 部署脚本
└── src         # c 语言写的部署脚本
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
