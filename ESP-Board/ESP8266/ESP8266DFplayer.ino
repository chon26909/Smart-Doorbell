#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//#define rxPin D1
//#define txPin D2
SoftwareSerial softwareSerialMP3 = SoftwareSerial(5, 4); // RX, TX
DFRobotDFPlayerMini DFPlayer;
void printDetail(uint8_t type, int value);

// WIFI
const char *ssid =  "IoT";
const char *password = "12345678";

// MQTT config
#define mqttServer "192.168.1.150"
#define mqttPort 1883

//song
int songIndex = 4;
int maxListSong = 4;
int volume = 16;
int timeToPlay = 2;

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

  if (String(topic) == "bell/stop") {
    DFPlayer.stop();
  }
  else if (String(topic) == "bell/volume")
  {
    if (messageTemp == "up") {
      if (volume < 30 ) {
        volume = volume + 2;
      }
    }
    if (messageTemp == "down") {
      if (volume > 0 ) {
        volume = volume - 2;
      }
    }
    DFPlayer.volume(volume);
  }
  else if (String(topic) == "bell/song")
  {
    if (messageTemp == "prev") {
      if (songIndex > 1) {
        songIndex--;
      }
    }
    if (messageTemp == "next") {
      if (songIndex < maxListSong) {
        songIndex++;
      }
      else {
        songIndex = 1;
      }
    }
  }
  else if (String(topic) == "bell/timeToPlay") {

    if (messageTemp == "+1") {
      timeToPlay++;
    }
    if (messageTemp == "-1") {
      timeToPlay--;
    }
  }
  else
  {
    Serial.print("ring bell");
    DFPlayer.play(songIndex);
    delay(timeToPlay * 1000);
  }

}

void connectWiFi()
{

  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

}

void setupMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
}

void reconnect()
{
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected())
  {

    Serial.println("Reconnecting to MQTT Broker..");

    if (mqttClient.connect("ESP8266BELL", "chon", "1234"))
    {
      Serial.println("Connected broker");
    }
  }
}

void setup()
{

  // define pin modes for tx, rx:
  pinMode(5, INPUT);
  pinMode(4, OUTPUT);

  //setting player
  pinMode(9, INPUT);    // prev song 
  pinMode(10, INPUT);   // next song
  pinMode(12, INPUT);   // volume up
  pinMode(13, INPUT);   // volume down
  pinMode(14, INPUT);   // stop player 

  Serial.begin(115200);
  softwareSerialMP3.begin(9600);

  connectWiFi();
  setupMQTT();
  if (!mqttClient.connected())
    reconnect();

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!DFPlayer.begin(softwareSerialMP3)) { //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  mqttClient.subscribe("bell/#");
  
  mqttClient.publish("ESP8266_MP3_CONNECTED", "true");
  
  DFPlayer.volume(volume); // volume 0 - 30  
  delay(200);
  DFPlayer.play(songIndex);
  delay(2000);
}

void loop()
{

  if (!mqttClient.connected()) reconnect();

  mqttClient.loop();

  if (digitalRead(9) == HIGH) {
    if (songIndex > 1) {
      songIndex--;
      DFPlayer.play(songIndex);
    }
  }
  if (digitalRead(10) == HIGH) {
    if (songIndex < maxListSong) {
      songIndex++;
      DFPlayer.play(songIndex);
      delay(timeToPlay * 1000);
    }
    else {
      songIndex = 1;
      DFPlayer.play(songIndex);
      delay(timeToPlay * 1000);
    }
  }
  if (digitalRead(14) == HIGH) {
    volume = volume - 2;
  }
  if (digitalRead(12) == HIGH) {
    volume = volume + 2;
  }
  if (digitalRead(13) == HIGH) {
    DFPlayer.stop();
  }
}
