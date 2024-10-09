<?php

if(isset($_GET['k'])) {
    $camera = $_GET['k'];
} else {
    $camera = 1;
}

$cam_data = '/home/www/asm/res_kam' . $camera . '.txt'; // Replace path of your file location on the server

if (file_exists($cam_data)) {
    $settings = file_get_contents($cam_data);
    $set = explode(",",$settings);
} else {
    $set = array('VGA',10,2,0,-2,0,100,1,0,0,0,0,);
}

$start = 0;

if (isset($_POST["framesize"])) {
    $set[0] = $_POST["framesize"];
    $start = 1;
}
if (isset($_POST["quality"])) {
    $set[1] = $_POST["quality"];
    $start = 1;
}
if (isset($_POST["brightness"])) {
    $set[2] = $_POST["brightness"];
    $start = 1;
}
if (isset($_POST["contrast"])) {
    $set[3] = $_POST["contrast"];
    $start = 1;
}
if (isset($_POST["saturation"])) {
    $set[4] = $_POST["saturation"];
    $start = 1;
}
if (isset($_POST["effect"])) {
    $set[5] = $_POST["effect"];
    $start = 1;
}
if (isset($_POST["aec_value"])) {
    $set[6] = $_POST["aec_value"];
    $start = 1;
}
if (isset($_POST["exposure_ctrl"])) {
    $set[7] = $_POST["exposure_ctrl"];
    $start = 1;
}
if (isset($_POST["gainceiling"])) {
    $set[8] = $_POST["gainceiling"];
    $start = 1;
}
if (isset($_POST["wb_mode"])) {
    $set[9] = $_POST["wb_mode"];
    $start = 1;
}
if (isset($_POST["colorbar"])) {
    $set[10] = $_POST["colorbar"];
    $start = 1;
}
if (isset($_POST["reset"])) {
    $set[11] = $_POST["reset"];
    $start = 1;
}

use Mosquitto\Client;

if ($start) {

    $username = "mosquitto"; // Replace with your MQTT brooker user name
    $password = "mosquitto"; // Replace with your MQTT brooker users password

    $host = "localhost";
    $port = 1883;
    $keepalive = 60;
    $interface = null;

    $topic = "cam" . $camera . "/in"; // Replace with your webserver topic name to send instructions from server to camera
    $payload = "";
    for ($i=0; $i < 12; ++$i) {
        $payload = $payload . $set[$i] . ",";
    }
    $set[11] = 0;
    file_put_contents($cam_data,$payload);
    $qos = 0;
    $retain = false;

    $client = new Mosquitto\Client("PHP02-sterowanie-kam" . $camera);
    // $client->setTlsInsecure(true);
    $client->setCredentials($username, $password);
    $client->onConnect(function() use ($client, &$mid, $topic, $payload, $qos, $retain) {
        $mid = $client->publish($topic, $payload, $qos, $retain);
        $client->exitLoop();
    });

    $client->connect($host, $port, $keepalive, $interface);
    $client->loopForever();
    $client->disconnect();

}

switch ($set[0]) {
    case 'QVGA':
        $width = 320;
        $height = 240;
        break;
    case 'CIF':
        $width = 352;
        $height = 288;
        break;
    case 'VGA':
        $width = 640;
        $height = 480;
        break;
    case 'SVGA':
        $width = 800;
        $height = 600;
        break;
    case 'XGA':
        $width = 1024;
        $height = 768;
        break;
    case 'SXGA':
        $width = 1280;
        $height = 1024;
        break;
    case 'UXGA':
        $width = 1600;
        $height = 1200;
        break;
}

echo '<html>

<head>

<style>

a: hover {
    background: #ffffff;
    text-decoration: none;
}
a {
    text-decoration: none;
    color: black;
}
a.p span {
    display: none;
    padding: 2px 3px;
    margin-left: 8px;
    width: 115px;
}
a.p:hover span{
    display: inline;
    position: absolute;
    margin: 15px 0 0 5px;
    background: #dddddd;
    color: #000000;
    -moz-opacity: 0.8;
    opacity:.80;
    filter: alpha(opacity=80);
    text-align: right;
}

</style>

</head>

<body>

<form style="display:inline-block;" action="/stream.php?k=' . $camera . '" method="POST">
<table>
    <tr>
    <td><label for="framesize" alt="framesize"><a class="p" href="#">resolution:<span>framesize</span></a></label></td>
    <td><select id="fra" name="framesize" style="width:90px;">
    <option value="QVGA"' . s("QVGA",$set[0]) . '>QVGA</option>
    <option value="CIF"' . s("CIF",$set[0]) . '>CIF</option>
    <option value="VGA"' . s("VGA",$set[0]) . '>VGA</option>
    <option value="SVGA"' . s("SVGA",$set[0]) . '>SVGA</option>
    <option value="XGA"' . s("XGA",$set[0]) . '>XGA</option>
    <option value="SXGA"' . s("SXGA",$set[0]) . '>SXGA</option>
    <option value="UXGA"' . s("UXGA",$set[0]) . '>UXGA</option>
    </select></td>
    </tr><tr>
    <td><label for="quality"><a class="p" href="#">compression:<span>quality (12-63)</span></a></label></td>
    <td><input type="text" id="qua" name="quality" value="' . $set[1] .'" style="width:90px;"></td>
    <tr><tr>
    <td><label for="">brightness:</label></td>
    <td><select id="bri" name="brightness" style="width:90px;">
    <option value="-2"' . s("-2",$set[2]) . '>-2</option>
    <option value="-1"' . s("-1",$set[2]) . '>-1</option>
    <option value="0"' . s("0",$set[2]) . '>0</option>
    <option value="1"' . s("1",$set[2]) . '>1</option>
    <option value="2"' . s("2",$set[2]) . '>2</option>
    </select></td>
    </tr></tr>
    <td><label for="contrast">contrast:</label></td>
    <td><select id="con" name="contrast" style="width:90px;">
    <option value="-2"' . s("-2",$set[3]) . '>-2</option>
    <option value="-1"' . s("-1",$set[3]) . '>-1</option>
    <option value="0"' . s("0",$set[3]) . '>0</option>
    <option value="1"' . s("1",$set[3]) . '>1</option>
    <option value="2"' . s("2",$set[3]) . '>2</option>
    </select></td>
    </tr><tr>
    <td><label for="saturation">saturation:</label></td>
    <td><select id="sat" name="saturation" style="width:90px;">
    <option value="-2"' . s("-2",$set[4]) . '>-2</option>
    <option value="-1"' . s("-1",$set[4]) . '>-1</option>
    <option value="0"' . s("0",$set[4]) . '>0</option>
    <option value="1"' . s("1",$set[4]) . '>1</option>
    <option value="2"' . s("2",$set[4]) . '>2</option>
    </select></td>
    </tr><tr>
    <td><label for="exposure_ctrl"><a class="p" href="#">exposure mode:<span>exposure_ctrl</span></a></label></td>
    <td><select id="exp" name="exposure_ctrl" style="width:90px;">
    <option value="0"' . s("0",$set[7]) . '>Manual</option>
    <option value="1"' . s("1",$set[7]) . '>Auto</option>
    </select></td>
    </select></td>
    </tr>';
if ($set[7] == 0) {
    echo '<tr>
    <td><label for="aec_value"><a class="p" href="#">exposure value:<span>aec_value (0-1200) - apply when exposure mode is manual</span></a></label></td>
    <td><input type="text" id="aec" name="aec_value" value="' . $set[6] . '" style="width:90px;"></td>
    <tr/>';
} else {
    echo '<input type="hidden" id="aec" name="aec_value" value="' . $set[6] . '">';
}
    echo '<tr>
    <td><label for="gainceiling"><a class="p" href="#">ISO like:<span>gainceiling (0-6)</span></a></label></td>
    <td><select id="gai" name="gainceiling" style="width:90px;">
    <option value="0"' . s("0",$set[8]) . '>0</option>
    <option value="1"' . s("1",$set[8]) . '>1</option>
    <option value="2"' . s("2",$set[8]) . '>2</option>
    <option value="3"' . s("3",$set[8]) . '>3</option>
    <option value="4"' . s("4",$set[8]) . '>4</option>
    <option value="5"' . s("5",$set[8]) . '>5</option>
    <option value="6"' . s("6",$set[8]) . '>6</option>
    </tr><tr>
    <td><label for="wb_mode"><a class="p" href="#">mode of white balace module:<span>wb_mode</span></a></label></td>
    <td><select id="wb_" name="wb_mode" style="width:90px;">
    <option value="0"' . s("0",$set[9]) . '>Auto</option>
    <option value="1"' . s("1",$set[9]) . '>Sunny</option>
    <option value="2"' . s("2",$set[9]) . '>Cloudy</option>
    <option value="3"' . s("3",$set[9]) . '>Office</option>
    <option value="4"' . s("4",$set[9]) . '>Home</option>
    </select></td>
    </tr><tr>
    <td><label for="effect"><a class="p" href="#">special effect:<span>efect</span></a></label></td>
    <td><select id="eff" name="effect" style="width:90px;">
    <option value="0"' . s("0",$set[5]) . '>No Effect</option>
    <option value="1"' . s("1",$set[5]) . '>Negative</option>
    <option value="2"' . s("2",$set[5]) . '>Grayscale</option>
    <option value="3"' . s("3",$set[5]) . '>Red Tint</option>
    <option value="4"' . s("4",$set[5]) . '>Green Tint</option>
    <option value="5"' . s("5",$set[5]) . '>Blue Tint</option>
    <option value="6"' . s("6",$set[5]) . '>Sepia</option>
    </select></td>
    </tr><tr>
    <td><label for="colorbar">colorbar:</label></td>
    <td><select id="col" name="colorbar" style="width:90px;">
    <option value="0"' . s("0",$set[10]) . '>Disable</option>
    <option value="1"' . s("1",$set[10]) . '>Enable</option>
    </select></td>
    </tr><tr>
    <td><label for="reset">hardware restart:</label></td>
    <td><select id="col" name="reset" style="width:90px;">
    <option value="0"' . s("0",$set[11]) . '>No action</option>
    <option value="1"' . s("1",$set[11]) . '>ESP32 restart</option>
    <option value="2"' . s("2",$set[11]) . '>Camera restart</option>
    </select></td>
    </tr><tr>
    <td></td>
    <td><input type="submit" value="Send" style="width:90px;"></td>
    </tr>
    </table>
</form>

<iframe src="webcam' . $camera . '.php" style="border:0;display:inline-block;" height="' . $height . '" width="' . $width . '" title="kam"' . $camera . '></iframe>

</body>

</html>';

function s($v,$t) {
    if ($v == $t) {
        $r = " selected";
    } else {
        $r = "";
    }
    return $r;
}

?>
