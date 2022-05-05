#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <WiFi.h>
#include <PubSubClient.h>

DFRobotDFPlayerMini DFPlayer;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// WIFI
const char *ssid =  "IoT";
const char *password = "12345678";

// MQTT config
#define mqttServer "192.168.1.150"
#define mqttPort 1883

//song
int songIndex = 4;
int maxListSong = 4;
int volume = 15;
int timeToPlay = 3;

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
        volume++;
      }
    }
    if (messageTemp == "down") {
      if (volume > 0 ) {
        volume--;
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
  else if(String(topic) == "bell/timeToPlay") { 

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
    DFPlayer.stop();
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
  Serial.begin(115200);
  Serial2.begin(9600);

  connectWiFi();
  setupMQTT();
  if (!mqttClient.connected())
    reconnect();

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!DFPlayer.begin(Serial2)) { //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  DFPlayer.volume(volume); // volume 0 - 30
  DFPlayer.play(3);

  mqttClient.subscribe("bell/#");
}

void loop()
{
  if (!mqttClient.connected()) reconnect();
  
  mqttClient.loop();

  if (digitalRead(12) == HIGH) {
    if (songIndex > 1) {
      songIndex--;
    }
  }
  if (digitalRead(13) == HIGH) {
    if (songIndex < maxListSong) {
      songIndex++;
    }
    else {
      songIndex = 1;
    }
  }
  if (digitalRead(14) == HIGH) {
    DFPlayer.stop();
  }
}
