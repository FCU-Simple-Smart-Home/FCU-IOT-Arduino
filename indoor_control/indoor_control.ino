/**
 * \file
 *       ESP8266 MQTT Bridge example
 * \author
 *       Tuan PM <tuanpm@live.com>
 */
#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include <QueueList.h>
#include "config.h"

SoftwareSerial debugPort(2, 3); // RX, TX
ESP esp(&debugPort, &Serial, 4); //debug mode here
//ESP esp(&debugPort, 4);
MQTT mqtt(&esp);

boolean wifiConnected = false;

QueueList<String> queue_topic;
QueueList<String> queue_payload;

int pin_no_leds[3] = {PIN_LIGHT_0, PIN_LIGHT_1, PIN_LIGHT_2};
bool status_leds[3] = {false, false, false};

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

void mqttConnected(void* response) {
    //subscribe
    Serial.println("Connected");
    mqtt.subscribe("led_0"); 
    mqtt.subscribe("led_1");
    mqtt.subscribe("led_2");
}

void mqttDisconnected(void* response)
{

}
//receive MQTT data here
void mqttData(void* response)                                   
{
    RESPONSE res(response);
    
    Serial.print("Received: topic=");
    String topic = res.popString();
    Serial.println(topic);
    
    Serial.print("data=");
    String data = res.popString();
    Serial.println(data);

    handleMqtt(topic, data);
}

void handleMqtt(String topic, String data) {
    if (topic == "led_0" || topic == "led_1" || topic == "led_2") {
        int index = topic[4] - '0';
        if (0 <= index && index <= 2) {
            if (data == "status") {
                queue_topic.push(topic);
                queue_payload.push(getStatusString(status_leds[index]));
            }
            else if (data == "on") {
                status_leds[index] = true;
                controlLed(pin_no_leds[index], status_leds[index]);
                queue_topic.push(topic);
                queue_payload.push(getStatusString(status_leds[index]));
            }
            else if (data == "off") {
                status_leds[index] = false;
                controlLed(pin_no_leds[index], status_leds[index]);
                queue_topic.push(topic);
                queue_payload.push(getStatusString(status_leds[index]));
            }
        }
    }
 
}

String getStatusString(bool flag) {
    return flag ? "status_on" : "status_off";
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
    while(!esp.ready());

    Serial.println("ARDUINO: setup mqtt client");
    if(!mqtt.begin("indoor_control_v2", "admin", "Isb_C4OGD4c3", 120, 1)) {
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
  
    //wificonfig
    esp.wifiConnect("FCU-Auto",""); 

    //pinMode declare
    pinMode(PIN_LIGHT_0, OUTPUT);
    pinMode(PIN_LIGHT_1, OUTPUT);
    pinMode(PIN_LIGHT_2, OUTPUT);
    
    controlLed(PIN_LIGHT_0, false);
    controlLed(PIN_LIGHT_1, false);
    controlLed(PIN_LIGHT_2, false);
    
    Serial.println("ARDUINO: system started");
}

void loop() {
    esp.process();
    if(wifiConnected) {
        while(!queue_topic.isEmpty()) {
            String topic = queue_topic.pop();
            String payload = queue_payload.pop();
            mqtt.publish((char *)topic.c_str(), (char *)payload.c_str());
        }
    }
}

void controlLed(int pin_no, bool flag) {
    if (flag) {
        digitalWrite(pin_no, LOW);
    }
    else {
        digitalWrite(pin_no, HIGH);
    }
}

