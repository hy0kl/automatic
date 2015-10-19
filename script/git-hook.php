<?php
/**
 * @describe:
 * @author: Jerry Yang(hy0kle@gmail.com)
 * */
$json = file_get_contents('php://input');
SeasLog::debug($json);

$git_data = json_decode($json, true);
if (! is_array($git_data)) {
    SeasLog::error('请求有误');
    exit;
}

$repository_name = $git_data['repository']['name'];
$ref_exp = explode('/', $git_data['ref']);
$exec_branch = array(
    'dev',
    'master',
    'develop',
);
if (! in_array($ref_exp[2], $exec_branch)) {
    $log = sprintf('当前分支不在待部署的配置中. [branch: %s]', $ref_exp[2]);
    SeasLog::error($log);
    exit;
}

$cmd = sprintf('./%s-deploy.sh', $repository_name);
if (! file_exists($cmd)) {
    $log = sprintf('部署脚本不存在: [%s]', $cmd);
    SeasLog::error($log);
    exit;
}

$output = system($cmd, $return_var);
$log = sprintf('[cmd: %s] [return_var: %s] [output: %s]', $cmd, $return_var, $output);
SeasLog::info($log);
/* vi:set ts=4 sw=4 et fdm=marker: */

