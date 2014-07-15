--TEST--
basic snowflake extension functionality test
--FILE--
<?php
$sf = new Snowflake();
$sf->connect('127.0.0.1', 8008);
$id = $sf->get();
var_dump(is_numeric($id));
$info = $sf->info();
var_dump(is_array($info));
var_dump($info['uptime'] > 0);
var_dump($info['ids'] > 0);
var_dump($sf);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
object(Snowflake)#1 (1) {
  ["socket"]=>
  resource(5) of type (Snowflake Socket Buffer)
}