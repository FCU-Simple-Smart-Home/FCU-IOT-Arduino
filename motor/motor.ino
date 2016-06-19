#include <Servo.h>
Servo myservo; 
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
  mqtt.subscribe(CHANNEL_DOOR); 
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
  myservo.attach(9);  
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);  
  while(!esp.ready());

  Serial.println("ARDUINO: setup mqtt client");
  if(!mqtt.begin("motor", "admin", "Isb_C4OGD4c3", 120, 1)) {
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


    if (topic.equals(CHANNEL_DOOR)) {
        if (payload.equals("on") ) 
        {
          open_door();
        }
        else if (payload.equals("off") ) 
        {
          close_door();
        }
        else if(payload.equals("status"))
        {
          mqtt.publish(CHANNEL_DOOR,door_status );
        }
    }

   
}

void open_door()
{
  if ( strcmp(door_status,"status_on") != 0){
      door_status = "status_on";
      mqtt.publish(CHANNEL_DOOR, door_status);
      Serial.println("open door");
      opendoor = true;
    }
}
void close_door()
{
  if ( strcmp(door_status,"status_off") != 0){
      door_status = "status_off";
      mqtt.publish(CHANNEL_DOOR, door_status);
      Serial.println("close door");
      opendoor = false;
    }
}


void loop(){
  esp.process();
  currentMillis = millis();
  if(wifiConnected) 
  {
   
  
  if ( opendoor)
  {
    if (currentMillis - leftpreviousMillis > 15)
     {
     // delay(15);
       if ( pos <= 180 )
       {
        myservo.write(pos);
        pos++   ;       
       }
       else
       leftpreviousMillis = millis(); 
     }
  }
  else
  {
    if (currentMillis - leftpreviousMillis > 15)
    {
     // delay(15);
      if ( pos >0 )
      {
        myservo.write(pos);
        pos--;          
      }
      else 
        leftpreviousMillis = millis();         
    }       
   }
 }
 }


