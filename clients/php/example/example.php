<?php 

require __DIR__ . "/../vendor/autoload.php";

$client = new Snowflake\Client;

var_dump($client->info());

var_dump($client->id());
var_dump($client->id());