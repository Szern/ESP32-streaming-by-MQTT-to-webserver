# ESP32-streaming-by-MQTT-to-webserver
Video streaming from ESP32-CAM to own webserver (for example VPS) using MQTT protocol with SSL. Complete solution.

## Things you need:
1. ESP32-CAM module & ESP32-CAM-MB USB Programmer, for example [this](https://sklep.msalamon.pl/produkt/plytka-esp32-z-kamera-esp32-cam-wifi-ble-4-2-dedykowany-programator/) or [this](https://www.amazon.de/s?k=Aideepen+ESP32-CAM+W-BT+Board+ESP32-CAM-MB+Micro+USB+to+Serial+Port+CH-340G+with+OV2640+2MP+Camera+Module+Dual+Mode+Support+NodeMCU)
2. A computer with an Arduino IDE installed
3. USB (computer) <-> MicroUSB (programer) cable
4. Acess to WiFi (ESP32-CAM).
5. Your own webserver (for example VPS). I'm using LEMP stack on Oracle Cloud Free Tier. In my case: Debian 11.9, nginx 1.18 and PHP 7.4.33 (FastCGI).

---

### Server side

Firstly you have to prepare MQTT brooker. I'am using Mosquitto MQTT Messaging Broker. Two good manuals:
* [How to Install and Secure the Mosquitto MQTT Messaging Broker on Debian 10](https://www.digitalocean.com/community/tutorials/how-to-install-and-secure-the-mosquitto-mqtt-messaging-broker-on-debian-10)
* [How to Install Mosquitto MQTT Message Broker on Debian 11](https://www.howtoforge.com/how-to-install-mosquitto-mqtt-message-broker-on-debian-11/)
In my case it is Debian 11.9 version.

Next you need to install MQTT client for PHP (exactly FastCGI wrapper). I'm using [Mosquitto-PHP](https://github.com/mgdm/Mosquitto-PHP)

Now you can start connection between ESP32-CAM and webserver.

### ESP32 side

In ArduinoIDE install [MQTTPubSubClient library](https://github.com/hideakitai/MQTTPubSubClient)

You can use file [/esp32cam/streamESP32CAM.ino](https://github.com/Szern/ESP32-streaming-by-MQTT-to-webserver/blob/main/esp32cam/streamESP32CAM.ino) but you have to change this lines:

```c
// configuration start

const char* ssid = "your_WiFi_network_name"; // Replace with your network name
const char* password = "password_to_your_WiFi_network_name"; // Replace with your password to WiFi

const char* ntpServer = "ntp1.tp.pl";
const long  gmtOffset_sec = 3600;   // Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 3600;  // Replace with your daylight offset (seconds)

const char* mqttServer = "www.server.eu"; // Replace with your webserver adress or webserver IP
const long mqttPort = 8883;
const char* mqttUser = "mosquitto"; // Replace with your MQTT brooker user name
const char* mqttPass = "mosquitto"; // Replace with your MQTT brooker users password
const char* mqttId = "esp32-cam";
const char* mqttTopicOut = "cam/out"; // Replace with your webserver topic name to send images from camera to server
const char* mqttTopicIn = "cam/in"; // Replace with your webserver topic name to send instructions from server to camera

const char CERT_CA[] =
  "-----BEGIN CERTIFICATE-----\n" \
// insert your sever CA certificate
// each line like to:
// "gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n" \
  "-----END CERTIFICATE-----\n";

// configuration end
```

After making this changes you upload file (using ArduinoIDE) to ESP32-CAM.

### Server scripts

On server we will build web interface. In nginx you have to configure domain (or subdomain) with SSL certyficate (for example free Certbot).
One file you need is [/webserver/webcam.php](https://github.com/Szern/ESP32-streaming-by-MQTT-to-webserver/blob/main/webserver/webcam.php).
You have to change three line:
```php
$username = "mosquitto"; // Replace with your MQTT brooker user name
$password = "mosquitto"; // Replace with your MQTT brooker users password
...
$topic = "cam/out"; // Replace with your webserver topic name to send images from camera to server
```
and second file: [/webserver/stream.php](https://github.com/Szern/ESP32-streaming-by-MQTT-to-webserver/blob/main/webserver/stream.php)
where you have to change the follow lines:
```php
$username = "mosquitto"; // Replace with your MQTT brooker user name
$password = "mosquitto"; // Replace with your MQTT brooker users password
...
$topic = "cam/in"; // Replace with your webserver topic name to send instructions from server to camera
```

---

That's all folks. You can connect power supply to ESP32-CAM and go to the browser (changing your domain or subdomain address): `https://www.yourdomain.com/stream.php`
