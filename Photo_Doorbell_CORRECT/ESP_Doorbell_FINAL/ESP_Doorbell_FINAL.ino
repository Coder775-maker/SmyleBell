#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <BlynkSimpleEsp32.h>
// RGB LED Pin definitions
#define RED 14
#define GREEN 13 
#define BLUE 12
// Select camera model
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
//#define LED 21
#define BUTTON 4
#define photo 14

//Variables
int i = 0;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
char auth[] = "text";
String st;
String content;
int resetbuttonpin = 16; 
//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);
String my_Local_IP;

//
void startCameraServer();

//Establishing Local server at port 80 whenever required
WebServer server(80);

// Used to capture photo.
void capture(int buttontype)
{
    uint32_t number = random(40000000);
    Serial.println("Capture " + buttontype);
    Blynk.setProperty(V1, "urls", "http://"+my_Local_IP+"/capture?_cb="+(String)number);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, HIGH);
    digitalWrite(RED, LOW);
    if (buttontype == 1){
      Blynk.notify("Someone is at the door..");
    }
    delay(1000);
    digitalWrite(BLUE, LOW);
}

//Fuctions used for WiFi credentials saving and connecting to it which you do not need to change 
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 60 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("WiFi connected");
    digitalWrite(GREEN, HIGH);
    digitalWrite(RED, LOW);
    digitalWrite(BLUE, LOW);
  }
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  Serial.println("SetupAP Yellow LED");
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("Smartbell Wi-Fi", "");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
 {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><br><br>";
      content += "<label>Password: </label><input name='pass' length=64><br><br>";
      content += "<label>Auth Token: </label><input name='auth' length=32><br><br>";
      content += "<input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qauth = server.arg("auth");
      if (qsid.length() > 0 && qpass.length() > 0 && qauth.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 128; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(qauth);
        Serial.println("");
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        Serial.println("writing eeprom auth:");
        for (int i = 0; i < qauth.length(); ++i)
        {
          EEPROM.write(96 + i, qauth[i]);
          Serial.print("Wrote: ");
          Serial.println(qauth[i]);
        }
        Serial.println("set flagByte 128 to one");
        EEPROM.write(128, 1);
        
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
    });
  } 
}

//Vitual button code for setting byteFlag at 128 eeprom location
/*
  BLYNK_WRITE(V2){
    int vbuttonstate = param.asInt();
    if (vbuttonstate == 1) {
      EEPROM.write(128, 0);
      EEPROM.commit();
      Serial.println("Flagbyte button pressed");
      ESP.restart();
    }
  }
  */
//End of functions

void setup() {
  Serial.begin(115200);
  //pinMode(LED,OUTPUT);
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
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
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
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

  #if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
  #endif
  //Wifi provisiong external code
  // RGB LED Pin setup
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  pinMode(resetbuttonpin, INPUT);
  //pinMode(photo, OUTPUT);
  //digitalWrite(photo, LOW);
  // END OF RBG LED PIN SETUP

  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  //pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  //Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  Serial.println("Reading EEPROM Auth");
  String eauth;
  for (int i = 96; i < 128; ++i){
    eauth += char(EEPROM.read(i));
  }
  int eauthlen = eauth.length();
  char eauth_array[eauthlen + 1];
  strcpy(eauth_array, eauth.c_str());
  Serial.print("AUTH: ");
  Serial.println(eauth_array);

  Serial.println("Read byteFlag at 128");
  int flagByte = EEPROM.read(128); 
  Serial.println(flagByte, DEC);
  
  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected to WiFi !!!");
    /* Delay of 3 mins for WiFi reconnection */
    int x = 0;
    while (x < 90){
      digitalWrite(GREEN, HIGH);
      digitalWrite(RED, HIGH);
      digitalWrite(BLUE, HIGH);
      delay(1000);
      digitalWrite(GREEN, LOW);
      digitalWrite(RED, LOW);
      digitalWrite(BLUE, LOW);
      delay(1000);
      x++;
    }
    //return;
    //delay(100);
    Serial.print(WiFi.localIP());
  }
  else if (flagByte == 1){
    Serial.println("flagByte is one so reset after 180 sec");
    /* Delay of 30 sec for WiFi reconnection */
    int x = 0;
    while (x < 15){
      digitalWrite(GREEN, HIGH);
      digitalWrite(RED, HIGH);
      digitalWrite(BLUE, HIGH);
      delay(1000);
      digitalWrite(GREEN, LOW);
      digitalWrite(RED, LOW);
      digitalWrite(BLUE, LOW);
      delay(1000);
      x++;
    }
    ESP.restart();
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");
  
  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print("NOTConnectedyet");
    delay(100);
    server.handleClient();
  }
  // END Wifi provisiong

  startCameraServer();
  
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  my_Local_IP = WiFi.localIP().toString();
  Serial.println("' to connect");
  
  Blynk.begin(eauth_array, esid.c_str(), epass.c_str());
  Blynk.setProperty(V1, "rotation", 90);
  Serial.println("Ending setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  int resetbuttonstate = digitalRead(resetbuttonpin);
    
  if (WiFi.status() == WL_CONNECTED){
    if(digitalRead(BUTTON) == HIGH){
      capture(1);
    }
    if (digitalRead(photo) == HIGH){
      capture(0);
    }
    digitalWrite(GREEN, HIGH);
    digitalWrite(RED, LOW);
    digitalWrite(BLUE, LOW);

  }
  else{
    Serial.println("Wifi Down Red LED");
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, LOW);
    delay(5000);
    ESP.restart();
  }

  if (resetbuttonstate == HIGH){
    EEPROM.write(128, 0);
    EEPROM.commit();
    Serial.println("Flagbyte button pressed");
    ESP.restart();
  }
  
}
