#!/bin/bash
# @describe:
# @author:   Jerry Yang(hy0kle@gmail.com)

#set -x

user=work
hosts_conf='192.168.64.10 192.168.64.11'
path="/home/work/higo/higo"

. deploy-git.sh deploy
