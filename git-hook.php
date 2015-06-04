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

