#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <MQTTPubSubClient.h>
#include "base64.h"

#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems

const char* ssid = "your_WiFi_network_name"; // Replace with your network name
const char* password = "password_to_your_WiFi_network_name"; // Replace with your password to WiFi

const char* ntpServer = "ntp1.tp.pl";
const long  gmtOffset_sec = 3600;   // Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 3600;  // Replace with your daylight offset (seconds)

const char* mqttServer = "www.server.eu"; // Replace with your webserver adress
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

// Pin definition for CAMERA_MODEL_AI_THINKER
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK     0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

WiFiClientSecure client;
MQTTPubSubClient mqtt;

void setClock() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.print("current time: ");
  time_t t = time(NULL);
  Serial.println(asctime(localtime(&t)));
}

void connect() {
connect_to_wifi:
    Serial.print("connecting to wifi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(" connected!");

setClock();

connect_to_host:
    Serial.print("connecting to host...");
    client.stop();
    client.setCACert(CERT_CA);
    while (!client.connect(mqttServer, mqttPort)) {
        Serial.print(".");
        delay(1000);
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected");
            goto connect_to_wifi;
        }
    }
    Serial.println(" connected!");

    Serial.print("connecting to mqtt broker...");
    mqtt.disconnect();
    while (!mqtt.connect(mqttId, mqttUser, mqttPass)) {
        Serial.print(".");
        delay(1000);
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected");
            goto connect_to_wifi;
        }
        if (client.connected() != 1) {
            Serial.println("WiFiClient disconnected");
            goto connect_to_host;
        }
    }
    Serial.println(" connected!");
}

void camera(String regulacja = "") {

  String rozdzielczosc = "QVGA", jakosc = "12", jasnosc = "1", kontrast = "2", nasycenie = "-2", efekty = "0", naswietlanie = "300", kontrolanaswietlania = "1", iso = "0", balansbieli = "0";

  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, atoi(jasnosc.c_str()));     // -2 to 2
  s->set_contrast(s, atoi(kontrast.c_str()));       // -2 to 2
  s->set_saturation(s, atoi(nasycenie.c_str()));     // -2 to 2
  s->set_special_effect(s, atoi(efekty.c_str())); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable  Set white balance
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable  Set white balance gain
  s->set_wb_mode(s, atoi(balansbieli.c_str()));        // 0 to 4 - The mode of white balace module. if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, atoi(kontrolanaswietlania.c_str()));  // 0 = disable , 1 = enable  Set exposure control
  s->set_aec2(s, 1);           // 0 = disable , 1 = enable  Auto Exposure Control
  s->set_ae_level(s, 0);       // -2 to 2  The auto exposure level to apply to the picture (when aec_mode is set to auto),
  s->set_aec_value(s, atoi(naswietlanie.c_str()));    // 0 to 1200 (300)  The Exposure value to apply to the picture (when aec_mode is set to manual), from 0 to 1200.
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30  The gain value to apply to the picture (when aec_mode is set to manual), from 0 to 30. Defaults to 0.
  s->set_gainceiling(s, (gainceiling_t)atoi(iso.c_str()));  // 0 to 6  The maximum gain allowed, when agc_mode is set to auto. This parameter seems act as “ISO” setting.
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // For tests purposes, it’s possible to replace picture get from sensor by a test color pattern.
  if (rozdzielczosc=="QVGA") {
    s->set_framesize(s, FRAMESIZE_QVGA);     // framebuffer size must be greater than or equal to the frame size, do not use sizes above QVGA when not JPEG, the performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates
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
  s->set_quality(s, atoi(jakosc.c_str())); // 10-63, for OV series camera sensors, lower number means higher quality
}

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //power up the camera if PWDN pin is defined
  if (CAM_PIN_PWDN != -1) {
    pinMode(CAM_PIN_PWDN, OUTPUT);
    digitalWrite(CAM_PIN_PWDN, LOW);
  }

  static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,
    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,
    .xclk_freq_hz = 20000000, //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    .fb_count = 2, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY // CAMERA_GRAB_WHEN_EMPTY//CAMERA_GRAB_LATEST. Sets when buffers should be filled
  };

  camera_config.fb_location = CAMERA_FB_IN_PSRAM; // CAMERA_FB_IN_DRAM || CAMERA_FB_IN_PSRAM;
  camera_config.frame_size = FRAMESIZE_UXGA;

  esp_err_t err = esp_camera_init(&camera_config); // initialize the camera
  mqtt.begin(client); // initialize mqtt client
  connect(); // connect to wifi, host and mqtt broker
  camera();

  mqtt.subscribe(mqttTopicIn, [](const String& payload, const size_t size) {
    Serial.println(payload);
    camera(payload);
  });

}

void loop() {

  mqtt.update(); // should be called

  if (!mqtt.isConnected()) {
      connect();
  }

  camera_fb_t * fb = esp_camera_fb_get(); // acquire a frame

  String imgDataB64 = base64::encode(fb->buf, fb->len);
  
  esp_camera_fb_return(fb); // return the frame buffer back to the driver for reuse
     
  mqtt.publish(mqttTopicOut, imgDataB64); // send image data

}
