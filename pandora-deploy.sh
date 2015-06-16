#!/bin/bash
# @describe:
# @author:   Jerry Yang(hy0kle@gmail.com)

#set -x

user=work
#hosts_conf='192.168.64.10 192.168.64.11'
hosts_conf='192.168.64.10'
path="/home/work/higo/higo"

. deploy-git.sh deploy

user=root
hosts_conf='192.168.190.2'
path="/opt/web/lehe.com/midian"

. deploy-git.sh deploy

path="/opt/web/ng-midian"
. deploy-git.sh deploy
