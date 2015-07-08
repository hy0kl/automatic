#!/bin/bash
# @describe:
# @author:   Jerry Yang(hy0kle@gmail.com)

#set -x

user=work
#hosts_conf='192.168.64.10 192.168.64.11'
hosts_conf='192.168.64.10'
path="/home/work/higo/higo "
. deploy-git.sh deploy

qas="/home/work/higo/higo-wenyy /home/work/higo/higo-liuyang /home/work/higo/higo-chelh /home/work/higo/higo-lichong /home/work/higo/higo-houbl"
for path in $qas
do
    . deploy-git.sh deploy
done

user=root
hosts_conf='192.168.190.2'
path="/opt/web/ng-midian"
. deploy-git.sh deploy
