
/**
 * \file
 *       ESP8266 MQTT Bridge example
 * \author
 *       Tuan PM <tuanpm@live.com>
 */
#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include "config.h"


SoftwareSerial debugPort(8, 9); // RX, TX
//ESP esp(&debugPort, &Serial, 4);
ESP esp(&debugPort, 4);
MQTT mqtt(&esp);

long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 5000;
boolean wifiConnected = false;

int lessthanrain = 500;
int lessthanlight = 500;
char* plug_0_status = "status_off";


String SensorTemp = "";                                                   //these  for MQTT & Sensor
String LED_0_status = "status_off";   
String LED_1_status = "status_off";
String LED_2_status = "status_off";
String LEDTemp = "";

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
      mqtt.connect("65.52.186.140", 1883, false);
      wifiConnected = true;
      //or mqtt.connect("host", 1883); /*without security ssl*/
    } else {
      wifiConnected = false;
      mqtt.disconnect();
    }

  }
}

void mqttConnected(void* response)                                                    //subscribe
{
  Serial.println("Connected");
  mqtt.subscribe("led_0"); //or mqtt.subscribe("topic"); /*with qos = 0*/
  mqtt.subscribe("led_1");
  mqtt.subscribe("led_2");
  mqtt.subscribe("marquee_message");
  mqtt.subscribe("door");
  mqtt.subscribe("sensor_window");
//  mqtt.subscribe("status");

 // mqtt.publish(CHANNEL_PLUG_0, plug_0_status);

}

void mqttDisconnected(void* response)
{

}
void mqttData(void* response)                                   //receive MQTT data here
{
  RESPONSE res(response);

  Serial.print("Received: topic=");
  String topic = res.popString();
  Serial.println(topic);

  Serial.print("data=");
  String data = res.popString();
  Serial.println(data);
  callback(topic,data);
}
void mqttPublished(void* response)
{

}
void setup() {                    //setup
  Serial.begin(19200);
  debugPort.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());
  
  pinMode(PIN_LIGHT_0,OUTPUT);                                               //pinMode declare
  pinMode(PIN_LIGHT_1,OUTPUT);
  pinMode(PIN_LIGHT_2,OUTPUT);

  Serial.println("ARDUINO: setup mqtt client");
  if(!mqtt.begin("indoor_control", "admin", "Isb_C4OGD4c3", 120, 1)) {
    Serial.println("ARDUINO: fail to setup mqtt");
    while(1);
  }


  Serial.println("ARDUINO: setup mqtt lwt");
  mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");

/*setup mqtt events */
  mqtt.connectedCb.attach(&mqttConnected);
  mqtt.disconnectedCb.attach(&mqttDisconnected);
  mqtt.publishedCb.attach(&mqttPublished);
  mqtt.dataCb.attach(&mqttData);

  /*setup wifi*/
  Serial.println("ARDUINO: setup wifi");
  esp.wifiCb.attach(&wifiCb);

  esp.wifiConnect("FCU-Auto","");                                                   //wificonfig


  Serial.println("ARDUINO: system started");
}

void callback(String topic, String payload) {                  //after receive MQTT data, find the sensor
    
    if(payload=="status") {
      SensorTemp = sensorStatus(topic);
      mqtt.publish(topic.c_str(), (char *)SensorTemp.c_str());
    }
    else{
      switchStatus(topic,payload);
      
    }
    
}


void switchStatus(String topic, String payload){                             //switch the status
  Serial.println("");
  if(payload == "on"){
    if(sensorStatus(topic) == "status_on"){
      Serial.println("already on");}
    digitalWrite(getLEDpin(topic), HIGH);
    setLEDStatuesWithPayload(topic , payload);
    mqtt.publish(topic.c_str(), (char *)sensorStatus(topic).c_str());
    Serial.println("[on] message sent");
    
  }
  else if(payload == "off"){
    if(sensorStatus(topic) == "status_off"){
      Serial.println("already off");}
    digitalWrite(getLEDpin(topic), LOW);
    setLEDStatuesWithPayload(topic , payload);
    mqtt.publish(topic.c_str(), (char *)sensorStatus(topic).c_str());
    Serial.println("[off] message sent");
    
  }
  else if(payload == "status_on"||payload == "status_off"){
    //debug & own message
    Serial.println("receive own message");
  }
  else
    Serial.println("Nothing happend");
    
  
}

String sensorStatus(String topic){                       //return Sensor Status
    if (topic.equals("led_0")) {
        return LED_0_status;
    }
    else if (topic.equals("led_1")) {
        return LED_1_status;
    }
    else if (topic.equals("led_2")) {
        return LED_2_status;
    }
    Serial.println("Sensor status error");
    return "error";
}

int getLEDpin(String topic){
   int LEDpin;
   if (topic.equals("led_0")) {
        LEDpin = PIN_LIGHT_0;
    }
    else if (topic.equals("led_1")) {
        LEDpin = PIN_LIGHT_1;
    }
    else if (topic.equals("led_2")) {
        LEDpin = PIN_LIGHT_2;
    }
  return LEDpin;
}

void setLEDStatuesWithPayload(String topic , String payload){
    String s = "";
    s = "status_";
    s += payload;
    if (topic.equals("led_0")) {
        LED_0_status = s;
    }
    else if (topic.equals("led_1")) {
        LED_1_status = s;
    }
    else if (topic.equals("led_2")) {
        LED_2_status = s;
    }
    else 
      Serial.println("Set status error");  
}

boolean body(){
/*  Serial.println("BODY : ");
  Serial.println(digitalRead(PIN_BODY));
  if (digitalRead(PIN_BODY)  == 1)
    return true;
  else
    return false;*/
}
void loop() {
  esp.process();
//  currentMillis = millis();
  if(wifiConnected) {
      
  }
}


