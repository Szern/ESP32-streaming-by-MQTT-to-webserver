<?php

use Mosquitto\Client;

$username = "mosquitto"; // Replace with your MQTT brooker user name
$password = "mosquitto"; // Replace with your MQTT brooker users password

$host = "localhost";
$port = 1883;
$keepalive = 60;
$interface = null;

$topic = "cam1/out"; // Replace with your webserver topic name to send images from camera to server
$qos = 0;

$client = new Mosquitto\Client("PHP1");
$client->setCredentials($username, $password);
$client->onMessage(function($message) use ($client) {
    $decodedData = base64_decode($message->payload);
    header("Cache-Control: no-cache");
    header("Pragma: no-cache");
    header("Refresh: 0;");
    header("Content-Type: image/jpeg");
    echo $decodedData;
    $client->exitLoop();
});

$client->connect($host, $port, $keepalive, $interface);
$client->subscribe($topic, $qos);

$client->loopForever(0);
$client->disconnect();

?>
