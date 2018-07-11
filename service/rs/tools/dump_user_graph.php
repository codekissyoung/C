
<?php
require_once('mysql.php');

$production = true; // 是否为生产环境

$mysql_ip = $production ? "10.10.39.3" : "10.10.61.57";
$redis_ip = $production ? "10.10.30.16" : "127.0.0.1";

$fname = "user_graph.txt";
$pre_filter_fname = "pre_filter_list.txt";
$post_filter_fname = "post_filter_list.txt";

# mysql 配置，用于初始化mysql中的测试数据
$db_conf = array(
    "host"=>$mysql_ip,
    "port"=>3306,
    "db_name"=>"qingyi",
    "charset"=>"utf8",
    "user"=>"root",
    "passwd"=>"iqingyi#ucloud",
);

$db = new Mysql();
if (!$db->init($db_conf, "qy_user")) {
    die("connect mysql failed\n");
}
$redis = new Redis();
$redis->connect($redis_ip);

echo "start\n";

$index = 0;
$len = 100;

$pf = fopen($fname, "w");
while (true) {
    $data = $db->select(array('uid', "fans_num", "all_post_num"), array(), array(), $index, $len);
    if (empty($data)) {
        break;
    }
    $result = "";
    foreach($data as $row) {
        $uid = $row->uid;
        $ret = $redis->zRange("follows:".$uid, 0, -1);
        $result .= $uid." ".$row->all_post_num." ".$row->fans_num." ".count($ret)."\n";
        foreach ($ret as $follow_id) {
            $result .= $follow_id."\n";
        }
        $result .= "\n";
        $index++;
    }
    fwrite($pf, $result);
}
echo "user num: ".$index."\n";
fclose($pf);

echo "get pre_filter_list\n";
$pf = fopen($pre_filter_fname, "w");
$ret = $redis->sMembers("pre_filter_list");
$result = "";
foreach ($ret as $uid) {
    $result .= $uid."\n";
}
fwrite($pf, $result);

echo "get post_filter_list\n";
$pf = fopen($post_filter_fname, "w");
$ret = $redis->sMembers("post_filter_list");
$result = "";
foreach ($ret as $uid) {
    $result .= $uid."\n";
}
fwrite($pf, $result);

