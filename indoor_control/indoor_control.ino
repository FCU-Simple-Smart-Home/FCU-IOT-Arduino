
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
  mqtt.subscribe("LED_0"); //or mqtt.subscribe("topic"); /*with qos = 0*/
  mqtt.subscribe("LED_1");
  mqtt.subscribe("LED_2");
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

  Serial.println("ARDUINO: setup mqtt client");
  if(!mqtt.begin("DVES_duino", "admin", "Isb_C4OGD4c3", 120, 1)) {
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

void callback(String topic, String payload) {                  //after receive MQTT data, find the sensor
    //debug


    if (topic.equals(LED_0)) {
        ledControlRelay(payload , 0);
    }
    else if (topic.equals(LED_1)) {
        ledControlRelay(payload , 1);
    }
   
}



void LEDcontrolRelay(String payload , int SWcontrol) {          //LED Number switch
    if (payload.equals("on") ) {
       LED_on(SWcontrol);
    }
    else if (payload.equals("off") ) {
        plug_0_off();
    }
     else if(payload.equals("status"))
    {
      mqtt.publish(CHANNEL_PLUG_0, plug_0_status);
    }
}

void LED_on(int SWcontrol){
  if(SWcontrol == 0){
    digitalWrite(PIN_LIGHT_0, HIGH);
  }
  else if(SWcontrol == 1){
    digitalWrite(PIN_LIGHT_0, HIGH);
  }
  else
    Serial.println("!!SWcontrol set on error!!");
    delay(10000);
  
}

void plug_0_on(){
  digitalWrite(PIN_PLUG_0, HIGH);
  plug_0_status = "status_on";
  mqtt.publish(CHANNEL_PLUG_0, plug_0_status);
  Serial.println("PLUG ON");
}

void plug_0_off(){
  digitalWrite(PIN_PLUG_0, LOW);
  plug_0_status = "status_off";
  mqtt.publish(CHANNEL_PLUG_0, plug_0_status);
  Serial.println("PLUG OFF");
}

boolean body(){
  Serial.println("BODY : ");
  Serial.println(digitalRead(PIN_BODY));
  if (digitalRead(PIN_BODY)  == 1)
    return true;
  else
    return false;
}
void loop() {
  esp.process();
  currentMillis = millis();
  if(wifiConnected) {
      if (rain())
      {
        plug_0_on();
      }
 
      else 
      {
        if(currentMillis - previousMillis > interval) 
        {
            previousMillis = currentMillis;
            plug_0_off();
        }
      }
  }
}


