<?php

class Mysql
{
    protected $_db_link = NULL;
    protected $_table_name = "";
    protected $_field_name = array();
    protected $_conf = NULL;
    protected $_field_map  = array();
    protected $_time_start = 0;

    public function init($conf, $table_name = "", $field_map = NULL)
    {
        $conf_name = array("host", "port", "user", "passwd", "charset", "db_name");
        if (!$this->check_conf($conf_name, $conf)) {
            return false;
        }

        $this->_db_link = mysqli_init();
        if ($this->_db_link === false) {
            return false;
        }

        # 设置自动重连
        # 设置不成功,php中不能设置自动重连
        //if ($this->_db_link->options(20/*MYSQL_OPT_RECONNECT*/, 1) === false) {
        //    return false;
        //}

        if ($this->_db_link->real_connect($conf['host'], $conf['user'],
                                          $conf['passwd'], $conf['db_name'], $conf['port']) === false) {
            return false;
        }
        

        /*
        $this->_db_link = new mysqli($conf['host'], $conf['user'], 
                                     $conf['passwd'], $conf['db_name'], $conf['port']);
        if (mysqli_connect_error()) {
            $this->_db_link = NULL;
            return false;
        }
         */

        if (!$this->_db_link->set_charset($conf['charset'])) {
            $this->free();
            return false;
        }

        if ($field_map) {
            $this->set_map($field_map);
        }
        if (array_key_exists("field_map", $conf)) {
            $this->set_map($conf['field_map']);
        }

        $this->_table_name = $table_name;
        $this->_conf = $conf;

        return true;
    }

    public function set_map($map)
    {
        $this->_field_map = $map;
    }

    public function insert($data, $map = array(), $table_name = "")
    {
        $this->escape($data);

        $table_name = $table_name ? $table_name : $this->_table_name;
        if ($table_name == "") {
            return FALSE;
        }
        if (empty($map)) {
            $map = $this->_field_map;
        }

        $sql = "INSERT INTO `".$table_name."` ";
        $sql_filed = "";
        $sql_value = "";
        foreach ($data as $key=>$value) {
            $new_key = $key;
            if (array_key_exists($key, $map)) {
                $new_key = $map[$key];
            }

            if ($sql_filed) {
                $sql_filed .= ",`".$new_key."`";
            }
            else {
                $sql_filed = "`".$new_key."`";
            }

            if ($sql_value) {
                $sql_value .= ",'".$value."'";
            }
            else {
                $sql_value = "'".$value."'";
            }
        }

        $sql = $sql . "($sql_filed) VALUES ($sql_value)";
        return $this->query($sql);
    }

    public function select($filed, $where, $order, $offset, $num, $map = array(), $table_name = "")
    {
        $this->escape($where);

        $table_name = $table_name ? $table_name : $this->_table_name;
        if ($table_name == "") {
            return FALSE;
        }

        if (empty($map)) {
            $map = $this->_field_map;
        }

        $sql = "SELECT ".$this->build_filed($filed, $map)." FROM ".
               $table_name ." ". $this->build_where($where, $map) ." ".
               $this->build_order($order, $map) ." ". $this->build_limit($offset, $num);

        $result = $this->query($sql);
        if ($result == FALSE) {
            return FALSE;
        }

        $data = array();
        while($obj = $result->fetch_object()){
            $data[] = $obj;
        }

        return $data;
    }
    
    public function like($filed, $where, $order, $offset, $num, $map = array(), $table_name = "")
    {
        $this->escape($where);

        $table_name = $table_name ? $table_name : $this->_table_name;
        if ($table_name == "") {
            return FALSE;
        }

        if (empty($map)) {
            $map = $this->_field_map;
        }

        $sql = "SELECT ".$this->build_filed($filed, $map)." FROM ".
               $table_name ." ". $this->build_like($where, $map) ." ".
               $this->build_order($order, $map) ." ". $this->build_limit($offset, $num);

        $result = $this->query($sql);
        if ($result == FALSE) {
            return FALSE;
        }

        $data = array();
        while($obj = $result->fetch_object()){
            $data[] = $obj;
        }

        return $data;
    }

    public function update($data, $where, $map = array(), $table_name = "") 
    {
        $this->escape($data);

        $table_name = $table_name ? $table_name : $this->_table_name;
        if ($table_name == "") {
            return FALSE;
        }
        
        if (empty($map)) {
            $map = $this->_field_map;
        }

        $sql = "UPDATE `".$table_name."`";
        $sql_sub = "";
        foreach ($data as $key=>$value) {
            $new_key = $key;
            if (array_key_exists($key, $map)) {
                $new_key = $map[$key];
            }

            $func = "";
            if (strlen($value) >= 5) {
                $func = substr($value, 0, 5);
            }

            if ($sql_sub) {
                if ($func == "INC()") {
                    $sql_sub .= ",`$new_key`=`$new_key`+1";
                }
                else if ($func == "DEC()") {
                    $sql_sub .= ",`$new_key`=`$new_key`-1";
                }
                else {
                    $sql_sub .= ",`$new_key`='$value'";
                }
            }
            else {
                if ($func == "INC()") {
                    $sql_sub .= "`$new_key`=`$new_key`+1";
                }
                else if ($func == "DEC()") {
                    $sql_sub .= "`$new_key`=`$new_key`-1";
                }
                else {
                    $sql_sub .= "`$new_key`='$value'";
                }
            }
        }

        $sql = $sql . " SET $sql_sub".$this->build_where($where, $map);
        return $this->query($sql);
    }

    public function delete($where, $map = array(), $table_name = "")
    {
        $this->escape($where);

        $table_name = $table_name ? $table_name : $this->_table_name;
        if ($table_name == "") {
            return FALSE;
        }

        if (empty($map)) {
            $map = $this->_field_map;
        }

        $sql = "DELETE FROM `".$table_name."`".$this->build_where($where, $map);
        return $this->query($sql);
    }

    public function query($sql)
    {
        $this->start_time();

        $result = @$this->_db_link->query($sql);

        # 如果mysql服务器端主动断开连接，则尝试重连，然后重新执行命令
        if ($result === false && ($this->_db_link->errno == 2006 || $this->_db_link->errno == 2013)) {

            # 要首先关闭原来的连接
            $this->free();

            # 重连
            $ret = $this->init($this->_conf, $this->_table_name);
            if ($ret == false) {
                return false;
            }

            # 重新执行命令
            $result = $this->_db_link->query($sql);
        }

        $ms = $this->end_time();

        if ($result === FALSE) {
        }

        return $result;
    }

    public function ping()
    {
        return $this->_db_link->ping();
    }

    public function free()
    {
        if ($this->_db_link) {
            $this->_db_link->close();
        }
    }

    protected function build_where($where, $map)
    {
        $sql = "";
        foreach ($where as $key=>$value) {

            if (array_key_exists($key, $map)) {
                $key = $map[$key];
            }

            if ($sql) {
                $sql .= "AND `$key` = '$value'";
            }
            else {
                $sql = "`$key` = '$value'";
            }
        }

        if ($sql) {
            return " WHERE ".$sql;
        }

        return $sql;
    }

    protected function build_like($where, $map)
    {
        $sql = "";
        foreach ($where as $key=>$value) {

            if (array_key_exists($key, $map)) {
                $key = $map[$key];
            }

            if ($sql) {
                $sql .= "AND `$key` LIKE '$value'";
            }
            else {
                $sql = "`$key` LIKE '$value'";
            }
        }

        if ($sql) {
            return " WHERE ".$sql;
        }

        return $sql;
    }

    protected function build_filed($fileds, $map)
    {
        if (empty($fileds)) {
            return "*";
        }

        $sql = "";
        foreach ($fileds as $key) {
            $filed = $key;
            if (array_key_exists($key, $map)) {
                $filed = $map[$key];
            }

            if ($sql) {
                $sql .= ", `$filed` AS $key";
            }
            else {
                $sql = "`$filed` AS $key";
            }
        }

        return $sql;
    }

    protected function build_order($order, $map)
    {
        if (empty($order)) {
            return "";
        }

        $sql = "";
        foreach ($order  as $filed=>$order_type) {
            if (array_key_exists($filed, $map)) {
                $filed = $map[$filed];
            }

            if ($sql) {
                $sql .= ", `$filed` $order_type";
            }
            else {
                $sql = "`$filed` $order_type";
            }
        }

        if ($sql) {
            $sql = "ORDER BY $sql";
        }

        return $sql;
    }

    protected function build_limit($offset, $num)
    {
        if ($num < 0) {
            return "LIMIT $offset";
        }

        return "LIMIT $offset, $num";
    }

    protected function escape(&$data)
    {
        foreach ($data as $key=>&$value) {
            $value = $this->_db_link->real_escape_string($value);
        }
    }

    protected function check_conf($conf_name, $conf)
    {
        foreach ($conf_name as $name) {
            if (!array_key_exists($name, $conf)) {
                return FALSE;
            }
        }

        return TRUE;
    }

    protected function start_time()
    {
        $this->_time_start = microtime();
    }

    protected function end_time()
    {
        list($old_usec, $old_sec) = explode(" ", $this->_time_start);
        list($now_usec, $now_sec) = explode(" ", microtime());

        return ($now_sec - $old_sec) * 1000 + ($now_usec - $old_usec) / 1000;
    }
};

