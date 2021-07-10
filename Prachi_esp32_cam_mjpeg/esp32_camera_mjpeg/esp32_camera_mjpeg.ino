/*

  This is a simple MJPEG streaming webserver implemented for AI-Thinker ESP32-CAM and
  ESP32-EYE modules.
  This is tested to work with VLC and Blynk video widget.

  Inspired by and based on this Instructable: $9 RTSP Video Streamer Using the ESP32-CAM Board
  (https://www.instructables.com/id/9-RTSP-Video-Streamer-Using-the-ESP32-CAM-Board/)

  Board: AI-Thinker ESP32-CAM

*/
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
 
//Ensure ESP32 Wrover Module
//Partial images will be transmitted if image exceeds buffer size
 
//camera model
#define CAMERA_MODEL_AI_THINKER
 
#include "camera_pins.h"
 
#define call_bell 4
#define photo 14
#define relay 12
//#define led 4
 
const char* ssid = "user"; // Enter your Wifi name
const char* password = "pass"; // Enter wifi password
char auth[] = "auth"; // Enter the Auth Token provied by Blynk app
 
String local_IP;
 
void startCameraServer();
 
void capture_photo()// This function will capture the photo
{
  //digitalWrite(led, HIGH);
  delay(200);
  uint32_t randomNum = random(50000);
  Serial.println("http://"+local_IP+"/capture?_cb="+ (String)randomNum);
  Blynk.setProperty(V1, "urls", "http://"+local_IP+"/capture?_cb="+(String)randomNum);
  //digitalWrite(led, LOW);
  delay(1000);
}
 
void setup() 
{
  Serial.begin(115200);
  pinMode(relay,OUTPUT);
  //pinMode(led,OUTPUT);
  Serial.setDebugOutput(true);
  Serial.println();
   
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
  // for larger pre-allocated frame buffer.
  if(psramFound()){
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
   
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
   
  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) 
  {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);
   
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
   
  startCameraServer();
   
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  local_IP = WiFi.localIP().toString();
  Serial.println("' to connect");
  Blynk.begin(auth, ssid, password);
}
 
void loop() // put your main code here, to run repeatedly
{
  Blynk.run();
  if(digitalRead(call_bell) == HIGH)
  {
    Serial.println("Send Notification to Blynk");
    Blynk.notify("Door Bell Pressed, Please Check"); // This notification will be sent to Blynk App
  }
  if(digitalRead(photo) == HIGH)
  {
    Serial.println("Capture Photo");
    capture_photo();
    delay(1000);
  }
  if(digitalRead(relay) == HIGH)
  {
    Serial.println("Unlock Door");
  }
}
