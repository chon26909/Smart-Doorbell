#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include <PubSubClient.h>
#define CAMERA_MODEL_AI_THINKER         
#include "camera_pins.h"

unsigned long timer1;                       // timer 1
unsigned long timer2;                       // timer 2
int coolDownTakePicture = 0;                // หน่วงเวลาในการถ่ายภาพครั้งถัดไป
int coolDownBell = 0;                       // หน่วงเวลาในการทำงานครั้งถัดไป 

// Flash
#define LED_BUILTIN 4
bool flash;                                 // ไฟ flash

// WIFI
const char *ssid = "Chon";                  // ssid WIFI
const char *password = "0837224629";        // password WIFI

// MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// MQTT config
const char *mqttServer = "192.168.1.150";   // mqtt server host
const int mqttPort = 1883;                  // mqtt server port
const int MAX_PAYLOAD = 60000;              // mqtt max patload

void startCameraServer();

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  Serial.println();

  if (String(topic) == "camera/capture")     // เมื่อมี topic "camera/capture" เข้ามา
  {
    take_picture();                          // capture face
  }
}

void connectWiFi()
{
  Serial.print("connecting to ");

  WiFi.begin(ssid, password);                // connect WIFI 

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
}

void setupMQTT()                             
{
  mqttClient.setServer(mqttServer, mqttPort); // connect MQTT Broker
  mqttClient.setCallback(callback);           // subscriber topic
}

void reconnect()
{
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected())
  {

    Serial.println("Reconnecting to MQTT Broker..");

    if (mqttClient.connect("ESP32CAM", "chon", "1234"))
    {
      Serial.println("Connected broker");
    }
  }
}

void take_picture()
{
  
  camera_fb_t *fb = NULL;
  if (true)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  };

  Serial.println("Taking picture");

  fb = esp_camera_fb_get(); // used to get a single picture.

  delay(100);
  flash = false;

  if (!fb)
  {
    Serial.println("Camera capture failed");
    return;
  }

  Serial.println("Picture taken");
  digitalWrite(LED_BUILTIN, LOW);
  sendPictureToMQTT(fb->buf, fb->len);
  esp_camera_fb_return(fb); // must be used to free the memory allocated by esp_camera_fb_get().
}

void sendPictureToMQTT(const uint8_t *buf, uint32_t len)
{
  Serial.println("Sending picture...");
  if (len > MAX_PAYLOAD)
  {
    Serial.print("Picture too large, increase the MAX_PAYLOAD value");
  }
  else
  {
    mqttClient.publish("camera/picture", buf, len, false);             
  }
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable detector
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("----------------------------------");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (psramFound())
  {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID)
  {
    s->set_vflip(s, 1);       // flip it back
    s->set_brightness(s, 1);  // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  connectWiFi();

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  setupMQTT();
  if (!mqttClient.connected())
    reconnect();

  pinMode(12, INPUT);
  pinMode(LED_BUILTIN, OUTPUT); // Define Flash as an output

  mqttClient.setBufferSize(MAX_PAYLOAD); // This is the maximum payload length
  mqttClient.subscribe("camera/capture");
}

void loop()
{
  if (!mqttClient.connected()) reconnect();
                                                                                                                                                                                               
  if (digitalRead(12) == HIGH) {
    
    if(coolDownBell == 0) {
       timer2 = millis();                                           // keep current time
       coolDownBell = 3;                                            // coolDownBell 3 second
       mqttClient.publish("bell/gate1", "true");                    // ring bell 
    }

    if (coolDownTakePicture == 0) {
      timer1 = millis();                                            // keep current time
      coolDownTakePicture = 10;                                     // coolDownTakePicture 10 second
      take_picture();                                               // capture
    }
  }

  if (millis() - timer1 >= 1000 && coolDownTakePicture > 0) {
    timer1 += 1000;
    timer1 = millis();
    coolDownTakePicture--;
    //Serial.print("coolDownTakePicture ");
    //Serial.println(coolDownTakePicture);
  }

  if (millis() - timer2 >= 1000 && coolDownBell > 0) {
    timer2 += 1000;
    timer2 = millis();
    coolDownBell--;
    //Serial.print("coolDownBell ");
    //Serial.println(coolDownBell);
  }

  mqttClient.loop();
}
