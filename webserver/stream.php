<?php

if (isset($_POST['resolution'])) {
	$res = $_POST["resolution"];
} else {
	$res = "QVGA";
}

use Mosquitto\Client;

$username = "mosquitto"; // Replace with your MQTT brooker user name
$password = "mosquitto"; // Replace with your MQTT brooker users password

$host = "localhost";
$port = 1883;
$keepalive = 60;
$interface = null;

$topic = "cam/in"; // Replace with your webserver topic name to send instructions from server to camera
$payload = $res;
$qos = 0;
$retain = false;

$c = new Mosquitto\Client("PHP3");

$c->setCredentials($username, $password);
$c->connect($host, $port, $keepalive, $interface );
$c->publish($topic, $payload, $qos, $retain);
$c->disconnect();

switch ($res) {
    case QVGA:
        $width = 320;
        $height = 240;
        $aqvga = " selected";
        break;
    case CIF:
        $width = 352;
        $height = 288;
        $bcif = " selected";
        break;
    case VGA:
        $width = 640;
        $height = 480;
        $cvga = " selected";
        break;
    case SVGA:
        $width = 800;
        $height = 600;
        $dsvga = " selected";
        break;
    case XGA:
        $width = 1024;
        $height = 768;
        $exga = " selected";
        break;
    case SXGA:
        $width = 1280;
        $height = 1024;
        $fsxga = " selected";
        break;
    case UXGA:
        $width = 1600;
        $height = 1200;
        $guxga = " selected";
        break;
}

echo '<html>

<head>
</head>

<body>

<div>
    <iframe src="webcam.php" style="border:none;" height="' . $height . '" width="' . $width . '" title="webcam"></ifram
</div>

<br /><br />
<div>
<form action="/stream.php" method="POST">
  <select id="res" name="resolution">
    <option value="QVGA"' . $aqvga . '>QVGA</option>
    <option value="CIF"' . $bcif . '>CIF</option>
    <option value="VGA"' . $cvga . '>VGA</option>
    <option value="SVGA"' . $dsvga . '>SVGA</option>
    <option value="XGA"' . $exga . '>XGA</option>
    <option value="SXGA"' . $fsxga . '>SXGA</option>
    <option value="UXGA"' . $guxga . '>UXGA</option>
  </select>
  <input type="submit" value="Submit">
</form>

</body>

</html>';

?>
