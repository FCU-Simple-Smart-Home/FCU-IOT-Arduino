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

SoftwareSerial debugPort(2, 3); // RX, TX
ESP esp(&debugPort, &Serial, 4);
MQTT mqtt(&esp);
boolean wifiConnected = false;

QueueList<String> queue_topic;
QueueList<String> queue_data;

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

void mqttConnected(void* response)
{
  Serial.println("Connected");
  mqtt.subscribe("led_0");
  mqtt.subscribe("led_1");
  mqtt.subscribe("led_2");
}
void mqttDisconnected(void* response)
{

}
void mqttData(void* response)
{
  RESPONSE res(response);

  Serial.print("Received: topic=");
  String topic = res.popString();
  Serial.println(topic);

  Serial.print("data=");
  String data = res.popString();
  Serial.println(data);

  if (topic == "led_0") {
    if (data == "status") {
      queue_topic.push("led_0");
      queue_data.push("status_on");
    }
  }
  else if (topic == "led_1") {
    if (data == "status") {
      queue_topic.push("led_1");
      queue_data.push("status_on");
    }
  }
  else if (topic == "led_2") {
    if (data == "status") {
      queue_topic.push("led_2");
      queue_data.push("status_on");
    }
  }
}
void mqttPublished(void* response)
{

}
void setup() {
  Serial.begin(19200);
  debugPort.begin(19200);
  Serial.println("ARDUINO: setup esp");
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  Serial.println("ARDUINO: wait esp");
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

void loop() {
  esp.process();
  if(wifiConnected) {
    while (!queue_topic.isEmpty()) {
      String topic = queue_topic.pop();
      String data = queue_data.pop();
      mqtt.publish((char *)topic.c_str(), (char *)data.c_str());
    }
  }
}
