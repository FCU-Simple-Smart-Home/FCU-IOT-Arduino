
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


SoftwareSerial debugPort(2, 3); // RX, TX
ESP esp(&debugPort, &Serial, 4);
//ESP esp(&debugPort, 4);
MQTT mqtt(&esp);




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
  mqtt.subscribe(CHANNEL_PLUG_0); //or mqtt.subscribe("topic"); /*with qos = 0*/
  mqtt.subscribe(CHANNEL_PLUG_1); 
}

void mqttDisconnected(void* response)
{

}

void mqttData(void* response)                                                       //receive message
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
void setup() {
  Serial.begin(19200);
  debugPort.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  pinMode(PIN_PLUG_0, OUTPUT);
  pinMode(PIN_PLUG_1, OUTPUT);
  while(!esp.ready());

  Serial.println("ARDUINO: setup mqtt client");
  if(!mqtt.begin("double_plug", "admin", "Isb_C4OGD4c3", 120, 1)) {
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

  esp.wifiConnect("FCU-Auto","");


  Serial.println("ARDUINO: system started");
}

void callback(String topic, String payload) {
    //debug


    if (topic.equals(CHANNEL_PLUG_0)) {
        if (payload.equals("on") ) {
          plug_on(0);
          stateflag = 1;
        }
        else if (payload.equals("off") ) {
          plug_off(0);
          stateflag = 0;
        }
        else if(payload.equals("status"))
        {
          mqtt.publish(CHANNEL_PLUG_0, plug_0_status);
        }
    }
    else if (topic.equals(CHANNEL_PLUG_1)) {
        if (payload.equals("on") ) {
          plug_on(1);
        }
        else if (payload.equals("off") ) {
          plug_off(1);
        }
        else if(payload.equals("status"))
        {
          mqtt.publish(CHANNEL_PLUG_1, plug_1_status);
        }
    }
   
}




boolean rain(){                 //取得是否有下雨 1為有
  
  int rain = digitalRead(PIN_RAIN_D0);
  if (raintemp != rain ){\
  //  Serial.println("rain : ");
    Serial.print(" ");
    Serial.print(rain);
    raintemp = rain;
  }
  if (raintemp == 0 ) //   ||  analogRead(PIN_RAIN_AO) <= lessthanrain
    return true;
  else 
    return false;
}

boolean light(){
  if (analogRead(PIN_LIGHTSENSOR) < lessthanlight)
    return true;
  else 
    return false;
}

boolean window(){
  if (digitalRead(PIN_WINDOW) == 0)
    return true;
  else 
    return false;
}

void plug_on(int number){
  if (number == 0)
  {
    if ( strcmp(plug_0_status,"status_off") == 0){
      digitalWrite(PIN_PLUG_0, HIGH);
      plug_0_status = "status_on";
      mqtt.publish(CHANNEL_PLUG_0, plug_0_status);
      Serial.println("PLUG1 ON");
    }
  }
  else if (number == 1)
  {
    if ( strcmp(plug_1_status,"status_off") == 0){
      digitalWrite(PIN_PLUG_1, HIGH);
      plug_1_status = "status_on";
      mqtt.publish(CHANNEL_PLUG_1, plug_1_status);
      Serial.println("PLUG2 ON");
    }
  }
  
}

void plug_off(int number){
  if (number == 0)
  {
    if ( strcmp(plug_0_status,"status_on") == 0){
      digitalWrite(PIN_PLUG_0, LOW);
      plug_0_status = "status_off";
      mqtt.publish(CHANNEL_PLUG_0, plug_0_status);
      Serial.println("PLUG1 OFF");
    }
  }
   else if (number == 1)
    {
      if ( strcmp(plug_1_status,"status_on") == 0){
        digitalWrite(PIN_PLUG_1, LOW);
        plug_1_status = "status_off";
        mqtt.publish(CHANNEL_PLUG_1, plug_1_status);
        Serial.println("PLUG2 OFF");
      }
   }
}

boolean body(){
  currentMillis = millis();
  if (currentMillis  > bodywait){
    int body = digitalRead(PIN_BODY);
    if (bodytemp != body){ 
      Serial.print("BODY : ");
      Serial.print(body);
      bodytemp = body;  
      if (body  == 1)
        return true;
      else
        return false;
  }
  }
  else
    return false;
}

void loop() {
  esp.process();
  currentMillis = millis();
  if(wifiConnected) {
   
      if (window() ||  rain() || body() || light() )
      {
        plug_on(0);
      }
      else 
      { 
        if(currentMillis - previousMillis > interval && stateflag == 0) // && strcmp(plug_0_status,"status_off")  == 0
        {
            previousMillis = currentMillis;
            plug_off(0);
        }
      }
  }
}


