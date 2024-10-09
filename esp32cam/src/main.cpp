// esp32mqtt camera software ver. 0.2 alfa

#include <Arduino.h>

#include <esp_system.h>
#include <esp_camera.h>
#include <soc/soc.h>            // Disable brownour problems
#include <soc/rtc_cntl_reg.h>   // Disable brownour problems
#include <esp_task_wdt.h>       // Watchdog

#include <WiFiClientSecure.h>
#include <MQTTPubSubClient.h>
#include <time.h>
#include <base64.h>
#include <nvs_flash.h>

#include <HttpsOTAUpdate.h>

#define WDT_TIMEOUT 45        // Watchdog Timeout in seconds
#define DEBUG_SERIAL 1         // 0 - off, 1 - init & frame, 2 - detailed
#define DEBUG_MQTT 1           // 0 - off, 1 - on

const int pin_flash = 4;
const int pin_led = 33;
const int pin_power = 32;

int mqtt_initialised = 0;
time_t t;
int ota = 0;
int counter;

// configuration start

const char* ssid = "your_WiFi_network_name"; // Replace with your network name
const char* password = "password_to_your_WiFi_network_name"; // Replace with your password to WiFi

static const char* url = "https://www.yourdomain.eu/cam/firmware.bin";

const char* ntpServer = "ntp1.tp.pl";
const long gmtOffset_sec = 3600;   // GMT offset (seconds)
const int daylightOffset_sec = 3600;  // daylight offset (seconds)

const char* mqttServer = "www.yourdomain.eu"; // Replace with your webserver adress or webserver IP
const long mqttPortS = 8883;
const long mqttPortU = 8884;
const char* mqttUser = "mosquitto"; // Replace with your MQTT brooker user name
const char* mqttPass = "mosquitto"; // Replace with your MQTT brooker users password
const char* mqttId = "esp32-cam1";
const char* mqttTopicOut = "cam1/out"; // Replace with your webserver topic name to send images from camera to server
const char* mqttTopicIn = "cam1/in"; // Replace with your webserver topic name to send instructions from server to camera
const char* mqttTopicDebug = "cam1/debug";
const char * mqtt_time = "time";
const char * mqtt_knock = "knock, knock";
const char * mqtt_memory = "memory";
const char * mqtt_signal = "signal";
const char * mqtt_echo = "echo";
const char * mqtt_ota = "runOTA";
const char * key1 = "settings"; // a place in flash memory
const char * key2 = "security"; // a place in flash memory
int security = 1;

// chain.pem
static const char * host_ca = "-----BEGIN CERTIFICATE-----\n"
"MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw\n"
"WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
"RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n"
"CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ\n"
"DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG\n"
"AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy\n"
"6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw\n"
"SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP\n"
"Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB\n"
"hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB\n"
"/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU\n"
"ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC\n"
"hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG\n"
"A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN\n"
"AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y\n"
"v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38\n"
"01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1\n"
"e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn\n"
"UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV\n"
"aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z\n"
"WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R\n"
"PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q\n"
"pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo\n"
"6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV\n"
"uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA\n"
"-----END CERTIFICATE-----\n";

// configuration end

WiFiClientSecure client;
MQTTPubSubClient mqtt;

esp_err_t err = nvs_flash_init();

void setClock();
void connect();
void camera(String rozkoduj = "");
void debug(String message, int line = 1, int mqtt = 1);
void memory_status();
int valid_command(String payload);
void HttpEvent(HttpEvent_t *event);

static HttpsOTAStatus_t otastatus;

void setup() { // ########################################################################################################

  if ( DEBUG_SERIAL != 0 ) {
    Serial.begin(115200);
    delay(500);
  }

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  esp_task_wdt_init(WDT_TIMEOUT, true); // Initialize ESP32 Task WDT
  esp_task_wdt_add(NULL);               // Subscribe to the Task WDT

  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led, LOW);
  // pinMode(pin_flash, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  client.setCACert(host_ca);
  mqtt.begin(client); // initialize mqtt client

  //client.setCertificate(test_client_cert); // for client verification
  //client.setPrivateKey(test_client_key);	// for client verification

// ### todo: server check, ask and take initial parameters

  connect(); // connect to wifi, host and mqtt broker
  // memory_status();

// Pin definition for CAMERA_MODEL_AI_THINKER
  static camera_config_t camera_config = {
    .pin_pwdn  = 32,
    .pin_reset = -1, //software reset will be performed
    .pin_xclk = 0,
    .pin_sccb_sda = 26,
    .pin_sccb_scl = 27,
    .pin_d7 = 35,
    .pin_d6 = 34,
    .pin_d5 = 39,
    .pin_d4 = 36,
    .pin_d3 = 21,
    .pin_d2 = 19,
    .pin_d1 = 18,
    .pin_d0 = 5,
    .pin_vsync = 25,
    .pin_href = 23,
    .pin_pclk = 22,

    .xclk_freq_hz = 20000000, //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode

    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,//YUV422 (2BPP/YUV422),GRAYSCALE (1BPP/GRAYSCALE),RGB565 (2BPP/RGB565),JPEG (JPEG/COMPRESSED)// PIXFORMAT_JPEG
    .frame_size = FRAMESIZE_UXGA, // framebuffer size must be greater than or equal to the frame size; FRAMESIZE_ + QVGA 320x240 || CIF 352x288 || VGA 640x480 || SVGA 800x600 || XGA 1024x768 || SXGA 1280x1024 || UXGA 1600x1200; for ESP32, do not use sizes above QVGA when not JPEG; the performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 10, // 10-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .fb_location = CAMERA_FB_IN_PSRAM, // CAMERA_FB_IN_DRAM (.frame_size: QVGA or CIF and .pixel_format: jpeg )|| CAMERA_FB_IN_PSRAM;
    .grab_mode = CAMERA_GRAB_LATEST // CAMERA_GRAB_WHEN_EMPTY - more control over the system, but results in longer time to get the frame // CAMERA_GRAB_LATEST Sets when buffers should be filled (.fb_count > 1), This approach puts more strain on the CPU/Memory, but allows for double the frame rate. Please use only with JPEG.
  };

  // define pin & power up the camera
  pinMode(pin_power, OUTPUT);
  digitalWrite(pin_power, LOW);
  esp_err_t err = esp_camera_init(&camera_config); // initialize the camera
  if (err != ESP_OK) { // przetestować, zmienić .fb_location przed OTA i w trybie awaryjnym
    pinMode(pin_power, OUTPUT); // After the camera uses the GPIO, the pinMode is reset. You need to set the pinMode again after the camera is turned off.
    digitalWrite(pin_power, HIGH);
    debug((char *)"Camera init failed! Restarting!");
    delay(500);
    pinMode(pin_power, OUTPUT);
    digitalWrite(pin_power, LOW);
    ESP.restart();
  }

  camera();

  digitalWrite(pin_led, HIGH);
  // digitalWrite(pin_flash, HIGH);
  // delay(2000);
  // digitalWrite(pin_flash, LOW);

}

void loop() { // ########################################################################################################

  if (ota) {
    otastatus = HttpsOTA.status();
    if (otastatus == HTTPS_OTA_SUCCESS) {
      esp_task_wdt_init(WDT_TIMEOUT, true);
      ota = 0;
      debug("Firmware written successfully.");
      // memory_status();
      ESP.restart();
    } else if (otastatus == HTTPS_OTA_FAIL) {
      esp_task_wdt_init(WDT_TIMEOUT, true);
      ota = 0;
      debug("Firmware Upgrade Fail");
    }
    delay(1000);
  } else {
    esp_task_wdt_reset();
    mqtt.update(); // should be called
    camera_fb_t * fb = esp_camera_fb_get(); // acquire a frame
    String imgDataB64 = base64::encode(fb->buf, fb->len);
    esp_camera_fb_return(fb); // return the frame buffer back to the driver for reuse
    mqtt.update();
    if ( !( (WiFi.status() == WL_CONNECTED) && mqtt.isConnected() && client.connected() ) ) {
      connect();
    }
    mqtt.publish(mqttTopicOut, imgDataB64, false, 0); // send image data (mqttTopicOut, imgDataB64, false, 0)
    debug(".",0);
    esp_task_wdt_reset();
  }

} // ###################################################################################################################

void setClock() { // ###################################################################################################

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  debug("Waiting for NTP time sync: ",0);
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(5000);
    debug(".",0);
    now = time(nullptr);
  }
  debug("current time: ",0);
  t = time(NULL);
  debug(asctime(localtime(&t)),0);
}

void connect() { // wifi, brooker mqtt ################################################################################

long mqttPort;

connect_to_wifi:
  if (WiFi.status() != WL_CONNECTED) {
    debug("connecting to WiFi...",0,0);
    esp_task_wdt_reset();
    while (WiFi.status() != WL_CONNECTED) {
      debug(".",0,0);
      delay(3000);
    }
    debug(" connected!",1,0);
    long rssi = WiFi.RSSI();
    char signal[4];
    sprintf(signal, "%d", rssi);
    debug("RSSI: ",0);
    debug(signal);
    esp_task_wdt_reset();
    setClock();
  }

connect_to_host:
  debug("connecting to MQTT host...",0,0);
  client.stop();
    if (security) {
    client.setCACert(host_ca);
    mqttPort = mqttPortS;
  } else {
    client.setInsecure(); // skip verification
    mqttPort = mqttPortU;
  }
  while (!client.connect(mqttServer, mqttPort)) {
    debug(".",0,0);
    delay(1000);
    if (WiFi.status() != WL_CONNECTED) {
      debug("WiFi disconnected");
      goto connect_to_wifi;
    }
  }
  debug(" connected!",1,0);
  esp_task_wdt_reset();

connect_to_brooker:
  debug("connecting to MQTT broker...",0,0);
  mqtt.disconnect();
  while (!mqtt.connect(mqttId, mqttUser, mqttPass)) {
    debug(".",0,0);
    delay(1000);
    if (WiFi.status() != WL_CONNECTED) {
      debug("WiFi disconnected",1,0);
      goto connect_to_wifi;
    }
    if (!client.connected()) {
      debug("MQTT host disconnected",1,0);
      goto connect_to_host;
    }
  }
  mqtt_initialised = 1;
  esp_task_wdt_reset();
  debug("connected to MQTT brooker!");

  // #### subscribe topic
  mqtt.subscribe(mqttTopicIn, [](const char * payload, const size_t size) {
    esp_task_wdt_reset();
    debug(payload);
    if ( strcmp(payload, mqtt_knock) != 0 )  {
      if ( strcmp(payload, mqtt_ota) == 0 ) {
        esp_task_wdt_deinit();
        HttpsOTA.onHttpEvent(HttpEvent);
        debug("Starting OTA");
        HttpsOTA.begin(url, host_ca); 
        debug("Will be patient, please. OTA update can take long, long time...");
        ota =1;
      } else if ( valid_command(payload) ) {
        camera(payload);
      } else if ( strcmp(payload, mqtt_echo) == 0 ) {
        debug("echo, echo, echo!");
      } else if ( strcmp(payload, mqtt_memory) == 0 ) {
        memory_status();
      } else if ( strcmp(payload, mqtt_time) == 0 ) {
        debug("current time: ",0);
        t = time(NULL);
        debug(asctime(localtime(&t)));
      } else if ( strcmp(payload, mqtt_signal) == 0 ) {
        for (int i = 0; i < 10; ++i) {
          long rssi = WiFi.RSSI();
          char signal[4];
          sprintf(signal, "%d", rssi);
          debug(signal);
          delay(1000);
        }
      } else if ( strcmp(payload, mqtt_lamp_on) == 0 ) {
        digitalWrite(pin_flash, HIGH);
      } else if ( strcmp(payload, mqtt_lamp_off) == 0 ) {
        digitalWrite(pin_flash, LOW);
      } else {
        debug("command not recognised");
      }
    }
  });
 
}

void camera(String rozkoduj) { // ####################################################################################

  String rozdzielczosc = "QVGA";
  int jakosc = 12, jasnosc = 1, kontrast = 2, nasycenie = -2, efekty = 0, naswietlanie = 300, kontrolanaswietlania = 1, iso = 0, balansbieli = 0, obraz_kontrolny = 0, lampa = 0;
  int status;
  if (rozkoduj[0] != '\0') {
    status = 0;

    int przerwa = ',';
    int i = 0, j = 0;
    char temp1[4];
    int temp2[4];

    int dlugosc = rozkoduj.length();
    rozkoduj[dlugosc] = '\0'; 
    
    while ((rozkoduj[i] != przerwa)) {
      temp1[i] = rozkoduj[i];
      ++i;
    }
    temp1[i] = '\0';
    rozdzielczosc = temp1;
    ++i;
    jakosc = 10 * (rozkoduj[i] - '0') + (rozkoduj[i+1] - '0');
    i = i + 3;
    if (rozkoduj[i] == '-') {
      jasnosc = -1 * (rozkoduj[i+1] - '0');
      i = i + 3;
    } else {
      jasnosc = rozkoduj[i] - '0';
      i = i + 2;
    }
    if (rozkoduj[i] == '-') {
      kontrast = -1 * (rozkoduj[i+1] - '0');
      i = i + 3;
    } else {
      kontrast = rozkoduj[i] - '0';
      i = i + 2;
    }
    if (rozkoduj[i] == '-') {
      nasycenie = -1 * (rozkoduj[i+1] - '0');
      i = i + 3;
    } else {
      nasycenie = rozkoduj[i] - '0';
      i = i + 2;
    }
    efekty = rozkoduj[i] - '0';
    i = i + 2;
    while ((rozkoduj[i] != przerwa)) {
      temp2[j] = rozkoduj[i] - '0';
      ++i;
      ++j;
    }
    for (int k = j - 1; k >= 0; --k) {
      naswietlanie = naswietlanie + temp2[k] * pow(10, j - k -1);
    }
    ++i;
    kontrolanaswietlania = rozkoduj[i] - '0';
    i = i + 2;
    iso = rozkoduj[i] - '0';
    i = i + 2;
    balansbieli = rozkoduj[i] - '0';
    i = i + 2;
    obraz_kontrolny = rozkoduj[i] - '0';
    i = i + 2;
//    lampa = rozkoduj[i] - '0';
//    i = i + 2;
//    if (lampa) {
//      digitalWrite(pin_flash, HIGH);
//    } else {
//      digitalWrite(pin_flash, LOW);
//    }
    if (rozkoduj[i] == '1') {
      debug((char *)"ESP32 restarting");
      ESP.restart();
    } else if (rozkoduj[i] == '2') {
      pinMode(pin_power, OUTPUT);
      digitalWrite(pin_power, HIGH);
      debug((char *)"Camera restarting");
      delay(500);
      pinMode(pin_power, OUTPUT);
      digitalWrite(pin_power, LOW);  // .Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.
    }
  } else {
    nvs_handle_t my_handle;
    err = nvs_open(key1, NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
      status = 2;
      esp_camera_load_from_nvs(key1); 
    } else {
      status = 1;
    }
  }
  if ( status < 2 ) {
    sensor_t * s = esp_camera_sensor_get();
    if (rozdzielczosc=="QVGA") {
      s->set_framesize(s, FRAMESIZE_QVGA);     // framsize have to be lower or equal than buffer declared; FRAMESIZE_ + QVGA = 5, VGA = 8, SVGA = 9, XGA = 10, SXGA = 12, UXGA = 13
    } else if (rozdzielczosc=="VGA") {
      s->set_framesize(s, FRAMESIZE_VGA);
    } else if (rozdzielczosc=="SVGA") {
      s->set_framesize(s, FRAMESIZE_SVGA);
    } else if (rozdzielczosc=="XGA") {
      s->set_framesize(s, FRAMESIZE_XGA);
    } else if (rozdzielczosc=="SXGA") {
      s->set_framesize(s, FRAMESIZE_SXGA);
    } else if (rozdzielczosc=="UXGA") {
      s->set_framesize(s, FRAMESIZE_UXGA);
    } else {
      s->set_framesize(s, FRAMESIZE_SVGA);
    }
    s->set_contrast(s, kontrast);       // -2 to 2
    s->set_brightness(s, jasnosc);     // -2 to 2
    s->set_saturation(s, nasycenie);     // -2 to 2
    s->set_gainceiling(s, (gainceiling_t)iso);  // 0 to 6  The maximum gain allowed, when agc_mode is set to auto. This parameter seems act as “ISO” setting.
    s->set_quality(s, jakosc); // 10-63, for OV series camera sensors, lower number means higher quality
    s->set_colorbar(s, obraz_kontrolny);       // For tests purposes, it’s possible to replace picture get from sensor by a test color pattern.
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable  Set white balance
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_exposure_ctrl(s, kontrolanaswietlania);  // 0 = disable , 1 = enable  Set exposure control
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // 0 = disable , 1 = enable

    s->set_aec2(s, 1);           // 0 = disable , 1 = enable  Auto Exposure Control
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable  Set white balance gain
    s->set_agc_gain(s, 0);       // 0 to 30  The gain value to apply to the picture (when aec_mode is set to manual), from 0 to 30. Defaults to 0.
    s->set_aec_value(s, naswietlanie);    // 0 to 1200 (300)  The Exposure value to apply to the picture (when aec_mode is set to manual), from 0 to 1200.

    s->set_special_effect(s, efekty); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_wb_mode(s, balansbieli);        // 0 to 4 - The mode of white balace module. if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_ae_level(s, 0);       // -2 to 2  The auto exposure level to apply to the picture (when aec_mode is set to auto),

    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable

    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    esp_camera_save_to_nvs(key1);
  }
}

void memory_status() {
  debug("PSRAM found: ",0);
  debug((String)psramFound());
  debug("Total heap: ",0);
  debug((String)ESP.getHeapSize());
  debug("Free heap: ",0);
  debug((String)ESP.getFreeHeap());
  debug("Total PSRAM: ",0);
  debug((String)ESP.getPsramSize());
  debug("Free PSRAM: ",0);
  debug((String)ESP.getFreePsram());
}

void debug(String message, int line, int mqt) {
  if ( DEBUG_SERIAL != 0 ) {
    if (line == 1) {
      Serial.println(message);
    } else {
      Serial.print(message);
    }
  }
  if ( mqt && ( DEBUG_MQTT != 0 ) && ( mqtt_initialised == 1 ) ) {
    mqtt.publish(mqttTopicDebug, message, false, 1);
  }
}

int valid_command(String payload) { // weak validate
  String letter = "QVSXU"; 
  int i = 0;
  int ret = 0;
  while (i < 5) {
    if ( payload[0] == letter[i] ) { 
      ret = 1;
    }
    i++;
  }
  return ret;
}

void HttpEvent(HttpEvent_t *event) {
  switch (event->event_id) {
    case HTTP_EVENT_ERROR:        Serial.println("Http Event Error"); break;
    case HTTP_EVENT_ON_CONNECTED: Serial.println("Http Event On Connected"); break;
    case HTTP_EVENT_HEADER_SENT:  Serial.println("Http Event Header Sent"); break;
    case HTTP_EVENT_ON_HEADER:    Serial.printf("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value); break;
    case HTTP_EVENT_ON_DATA:      break;
    case HTTP_EVENT_ON_FINISH:    Serial.println("Http Event On Finish"); break;
    case HTTP_EVENT_DISCONNECTED: Serial.println("Http Event Disconnected"); break;
//    case HTTP_EVENT_REDIRECT:     Serial.println("Http Event Redirect"); break;
  }
}
