// Dual channel blinds with DHT22 temp sensor
// positionState topic used to define boot position
// For the first time usage, a position of 0 must be published on the /positionState/1 and /positionState/2 topics

#include <PubSubClient.h>
#include <SimpleTimer.h>    //https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer
#include <ESP8266WiFi.h>    //if you get an error here you need to install the ESP8266 board manager 
#include <ESP8266mDNS.h>    //if you get an error here you need to install the ESP8266 board manager 
#include <PubSubClient.h>   //https://github.com/knolleary/pubsubclient
#include <ArduinoOTA.h>     //https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA
#include "DHT.h"
#include <ArduinoJson.h>
#include <AccelStepper.h>
#include <LocalLanStuff.h>  // This file contains the wifi info

/*****************  START USER CONFIG SECTION *********************************/
/*****************  START USER CONFIG SECTION *********************************/

#define USER_MQTT_CLIENT_NAME     "BlindsMCU1"         // Used to define MQTT Client ID, and ArduinoOTA

#define STEPS_TO_CLOSE            650                  //Defines the number of steps needed to open or close fully

#define dirPin1 D6
#define stepPin1 D7
#define enablePin1 D5
#define dirPin2 D8
#define stepPin2 D2
#define enablePin2 D1
#define motorInterfaceType 1

#define DHTPIN 9      // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
 
/*****************  END USER CONFIG SECTION *********************************/
/*****************  END USER CONFIG SECTION *********************************/


WiFiClient espClient;
PubSubClient client(espClient);
SimpleTimer timer;
DHT dht1(DHTPIN, DHTTYPE);
AccelStepper stepper1 = AccelStepper(motorInterfaceType, stepPin1, dirPin1);
AccelStepper stepper2 = AccelStepper(motorInterfaceType, stepPin2, dirPin2);

//Global Variables
bool boot1 = true;
bool boot2 = true;
bool moving1 = false;
bool moving2 = false;

const char* ssid = USER_SSID ; 
const char* password = USER_PASSWORD ;
const char* mqtt_server = USER_MQTT_SERVER ;
const int mqtt_port = USER_MQTT_PORT ;
const char *mqtt_user = USER_MQTT_USERNAME ;
const char *mqtt_pass = USER_MQTT_PASSWORD ;
const char *mqtt_client_name = USER_MQTT_CLIENT_NAME ; 

//Functions
void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() 
{
  int retries = 0;
  while (!client.connected()) {
    if(retries < 150)
    {
      Serial.print("Attempting MQTT connection...");
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) 
      {
        Serial.println("connected");
        if(boot1 == false || boot2 == false)
        {
          client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Reconnected"); 
        }
        if(boot1 == true && boot2 == true)
        {
          client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Rebooted");
        }
        // ... and resubscribe
        client.subscribe(USER_MQTT_CLIENT_NAME"/blindsCommand/1");
        client.subscribe(USER_MQTT_CLIENT_NAME"/positionCommand/1");
        client.subscribe(USER_MQTT_CLIENT_NAME"/blindsCommand/2");
        client.subscribe(USER_MQTT_CLIENT_NAME"/positionCommand/2");
        client.subscribe(USER_MQTT_CLIENT_NAME"/positionState/1");
        client.subscribe(USER_MQTT_CLIENT_NAME"/positionState/2");
      } 
      else 
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if(retries > 149)
    {
    ESP.restart();
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  char charPayload[50];
  char positionPublish[50];
  Serial.print("Message arrived [");
  String newTopic = topic;
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  int intPayload = newPayload.toInt();
  Serial.println(newPayload);
  Serial.println();
  newPayload.toCharArray(charPayload, newPayload.length() + 1);
  
  if (newTopic == USER_MQTT_CLIENT_NAME"/blindsCommand/1") 
  {
    if (newPayload == "OPEN")
    {
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand/1", "0");
    }
    else if (newPayload == "CLOSE")
    {   
      int stepsToClose = STEPS_TO_CLOSE;
      String temp_str = String(stepsToClose);
      temp_str.toCharArray(charPayload, temp_str.length() + 1);
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand/1", charPayload);
    }
    else if (newPayload == "STOP")
    {
      stepper1.stop();
      String temp_str = String(stepper1.currentPosition());
      temp_str.toCharArray(positionPublish, temp_str.length() + 1);
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand/1", positionPublish); 
    }
  }
  if (newTopic == USER_MQTT_CLIENT_NAME"/blindsCommand/2") 
  {
    if (newPayload == "OPEN")
    {
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand/2", "0");
    }
    else if (newPayload == "CLOSE")
    {   
      int stepsToClose = STEPS_TO_CLOSE;
      String temp_str = String(stepsToClose);
      temp_str.toCharArray(charPayload, temp_str.length() + 1);
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand/2", charPayload);
    }
    else if (newPayload == "STOP")
    {
      stepper2.stop();
      String temp_str = String(stepper2.currentPosition());
      temp_str.toCharArray(positionPublish, temp_str.length() + 1);
      client.publish(USER_MQTT_CLIENT_NAME"/positionCommand/2", positionPublish); 
    }
  }
  
  if (newTopic == USER_MQTT_CLIENT_NAME"/positionCommand/1")
  {
    if(boot1 == false)
    {
        stepper1.enableOutputs();
        stepper1.moveTo(intPayload);
        stepper1.setSpeed(400);
        moving1 = true;
    }
  }
  if (newTopic == USER_MQTT_CLIENT_NAME"/positionCommand/2")
  {
    if(boot2 == false)
    {
        stepper2.enableOutputs();
        stepper2.moveTo(intPayload);
        stepper2.setSpeed(400);
        moving2 = true;
    }
  }
  
  if (newTopic == USER_MQTT_CLIENT_NAME"/positionState/1")
  { 
    if(boot1 == true)
    {
      client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Got initial positionState 1");
      stepper1.setCurrentPosition(intPayload);
      client.unsubscribe(USER_MQTT_CLIENT_NAME"/positionState/1");
      boot1 = false;
    }
  }
  if (newTopic == USER_MQTT_CLIENT_NAME"/positionState/2")
  { 
    if(boot2 == true)
    {
      client.publish(USER_MQTT_CLIENT_NAME"/checkIn","Got initial positionState 2");
      stepper2.setCurrentPosition(intPayload);
      client.unsubscribe(USER_MQTT_CLIENT_NAME"/positionState/2");
      boot2 = false;
    }
  }
}

void checkIn()
{
  client.publish(USER_MQTT_CLIENT_NAME"/checkIn","OK"); 
}

void getTemp()
{
  float p_humidity = dht1.readHumidity();
  float p_temperature = dht1.readTemperature();
  if (isnan(p_humidity) || isnan(p_temperature))
  {
    client.publish(USER_MQTT_CLIENT_NAME"/TempSensor/status", "ERROR: Failed to read from DHT sensor!");
    return;
  }
  else
  {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    char str[32];
    snprintf(str, sizeof(str), "%.1f", p_temperature);
    root["temperature"] = str;
    snprintf(str, sizeof(str), "%.0f", p_humidity);
    root["humidity"] = str;
    root.prettyPrintTo(Serial);
    Serial.println("");
    char data[200];
    root.printTo(data, root.measureLength() + 1);
    client.publish(USER_MQTT_CLIENT_NAME"/TempSensor", data, true);
  }
}

void getPosition()
{
  char positionPublish[50];
  String temp_str;
  temp_str = String(stepper1.currentPosition());
  temp_str.toCharArray(positionPublish, temp_str.length() + 1);
  client.publish(USER_MQTT_CLIENT_NAME"/stepperCurrentPosition/1", positionPublish); 
  temp_str = String(stepper2.currentPosition());
  temp_str.toCharArray(positionPublish, temp_str.length() + 1);
  client.publish(USER_MQTT_CLIENT_NAME"/stepperCurrentPosition/2", positionPublish); 
}


//Run once setup
void setup() {
  Serial.begin(115200);

  stepper1.setEnablePin(enablePin1);
  stepper1.setPinsInverted(true, false, true);
  stepper1.disableOutputs();
  stepper1.setMaxSpeed(500);
  stepper1.setSpeed(500);
  stepper1.setAcceleration(500);

  stepper2.setEnablePin(enablePin2);
  stepper2.setPinsInverted(true, false, true);
  stepper2.disableOutputs();
  stepper2.setMaxSpeed(500);
  stepper2.setSpeed(500);
  stepper2.setAcceleration(500);
  
  WiFi.mode(WIFI_STA);
  setup_wifi();

  dht1.begin();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  ArduinoOTA.setHostname(USER_MQTT_CLIENT_NAME);
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n OTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
  });
  ArduinoOTA.begin(); 
  Serial.println("OTA ready");
  delay(10);
  timer.setInterval(120000, checkIn);
//  timer.setInterval(10000, getPosition);
  timer.setInterval(300000, getTemp);
}

void loop()
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
  timer.run();
  
  stepper1.runSpeedToPosition();
  if(moving1 == true && stepper1.distanceToGo() == 0)
  {
    char positionPublish[50];
    String temp_str = String(stepper1.currentPosition());
    temp_str.toCharArray(positionPublish, temp_str.length() + 1);
    client.publish(USER_MQTT_CLIENT_NAME"/positionState/1", positionPublish, true);
    stepper1.disableOutputs();
    moving1 = false;
  }

  stepper2.runSpeedToPosition();
  if(moving2 == true && stepper2.distanceToGo() == 0)
  {
    char positionPublish[50];
    String temp_str = String(stepper2.currentPosition());
    temp_str.toCharArray(positionPublish, temp_str.length() + 1);
    client.publish(USER_MQTT_CLIENT_NAME"/positionState/2", positionPublish, true);
    stepper2.disableOutputs();
    moving2 = false;
  }
}
