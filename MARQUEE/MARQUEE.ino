#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include "config.h"
#include <MaxMatrix.h>
#include <avr/pgmspace.h>
#include <IRremote.h>
#include <IRremoteInt.h>
IRsend irsend;

#include <stdio.h>
#include <DS1302.h>
/**************************************************************************************時間********************************************************************************************/
decode_results results;

namespace {
DS1302 rtc(PIN_RST, PIN_IO, PIN_CLK);

           //namespace


MaxMatrix m(8, 9, 10, 3); // define module





IRrecv irrecv(PIN_REC); 

SoftwareSerial debugPort(13, 14); // RX, TX
ESP esp(&debugPort, &Serial, 4);
//ESP esp(&debugPort, 4);
MQTT mqtt(&esp);



void printTime() {
  Time t = rtc.time();
  char buf[50];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
           t.hr, t.min, t.sec);
  for (int i = 0 ; i < 25 ; i++)
  {
    TIME[i] = buf[i];
  }
}
}






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
  mqtt.subscribe(CHANNEL_TEMPERATURE);
  mqtt.subscribe(CHANNEL_MESSAGE);
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
  printTime();
/**************************************************************************************時間********************************************************************************************/
  rtc.writeProtect(false);
  rtc.halt(false);
  Time t();
  
  rtc.time();
  /**************************************************************************************時間********************************************************************************************/
    
  m.init(); // module initialize
  m.setIntensity(0); // dot matix intensity 0-15
  Serial.begin(19200); // serial communication initialize
/**************************************************************************************LED矩陣跑馬燈********************************************************************************************/  
  while(!esp.ready());

  Serial.println("ARDUINO: setup mqtt client");
  if(!mqtt.begin("LEDARRAY", "admin", "Isb_C4OGD4c3", 5000, 1)) {
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
  if (topic.equals(CHANNEL_MESSAGE)   ) 
  {
    nowelement = 0;
    delay(100);
    m.shiftLeft(false, false);
    havemessage = true;
    strcpy(MESSAGE, payload.c_str());
    Serial.println(MESSAGE);
    previousMillis = millis();

  }
  
  else if (topic.equals(CHANNEL_TEMPERATURE))
  {
   //  if (!presenttemperture.equals(payload) )
  //   {
    //   presenttemperture = String(payload);
       strcpy(TEMPERA, payload.c_str());
       Serial.println(TEMPERA);        
    // }
  }
           
}




void loop() {
  esp.process();
  
  currentMillis = millis();
  if(wifiConnected) {
    
    
      
  }

  if (havemessage)                         //有訊息的話
  {
    if (currentMillis - previousMillis > 10000)
    {
      havemessage = false;
      Serial.println("hasmessage end");
      previousMillis = millis();
      delay(100);
      m.shiftLeft(false, false);
    }

    if (nowbuf < buffer[0]+1)
      {
       if (currentMillis - leftpreviousMillis > 100)
        {
         // delay(100);
          m.shiftLeft(false, false);
          nowbuf ++;
        /*  Serial.print(nowbuf);
          Serial.print(" ?= ");
          Serial.print(buffer[0]+1);
          Serial.println( );*/
          leftpreviousMillis = millis();
        }
      }
      else if ( MESSAGE[nowelement] == 0 )
      {
        nowelement =0;
       
         delay(100);
         memcpy_P(buffer, CH + 7*0, 7);
         m.writeSprite(32, 0, buffer);
         m.shiftLeft(false, false);
          leftpreviousMillis = millis();
        
      }
      else if ( nowelement < sizeof(MESSAGE))
      {
        
        if (MESSAGE[nowelement] == '\0')
          {
             
            nowelement = 0;
            memcpy_P(buffer, CH + 7*0, 7);
            m.writeSprite(32, 0, buffer);
           }
        else if (MESSAGE[nowelement] >= 32) 
        {
          //MESSAGE[nowelement] -= 32;
          memcpy_P(buffer, CH + 7*(MESSAGE[nowelement]-32), 7);
          m.writeSprite(32, 0, buffer);
          m.setColumn(32 + buffer[0], 0);
           
          nowelement++;
          nowbuf = 0;
    
        }
        else
        {
          nowelement++;
        }
     }
    
  }
  else                                       //訊息結束
  {
    if (currentMillis - previousMillis > 10000)
    {
      printTime();
      Serial.println(TIME);
      nowelement = 0;
      nowbuf = 9999;
      previousMillis = currentMillis;

      
    }
    else if (currentMillis - previousMillis < 4000 )                  //4秒顯示時間
    {
      if (nowbuf < buffer[0]+1)
      {
       if (currentMillis - leftpreviousMillis > 100)
        {
         // delay(100);
          m.shiftLeft(false, false);
          nowbuf ++;
          /*Serial.print(nowbuf);
          Serial.print(" ?= ");
          Serial.print(buffer[0]+1);
          Serial.println( );*/
          leftpreviousMillis = millis();
        }
      }
      else if ( nowelement < sizeof(TIME))
      {
        if (TIME[nowelement] == '\0')
          {
             
            nowelement = 0;
            memcpy_P(buffer, CH + 7*0, 7);
            m.writeSprite(32, 0, buffer);
            
           }
        else if (TIME[nowelement] >= 32) 
        {
        
  
          
      
          TIME[nowelement] -= 32;
          memcpy_P(buffer, CH + 7*TIME[nowelement], 7);
          m.writeSprite(32, 0, buffer);
          m.setColumn(32 + buffer[0], 0);
           
          nowelement++;
          nowbuf = 0;
    
        }
        else
        {
          nowelement++;
        }
          
      
    }
    
    }                                             //顯示時間結束
    else if ( currentMillis - previousMillis > 5000  && currentMillis - previousMillis < 9000 )  //顯示溫度開始
    {
     
      if (nowbuf < buffer[0]+1)
      {
       if (currentMillis - leftpreviousMillis > 100)
        {
         // delay(100);
          m.shiftLeft(false, false);
          nowbuf ++;
        /*  Serial.print("tmpebuf:");
          Serial.print(" ?= ");
          Serial.print(buffer[0]+1);
          Serial.println( );*/
          leftpreviousMillis = millis();
        }
      }
      else if ( TEMPERA[nowelement] == 0 )
      {
       if (currentMillis - leftpreviousMillis > 100)
        {
         // delay(100);
         memcpy_P(buffer, CH + 7*0, 7);
         m.writeSprite(32, 0, buffer);
         m.shiftLeft(false, false);
          leftpreviousMillis = millis();
        }
      }
      else if ( nowelement < sizeof(TEMPERA))
      {
        int i = TEMPERA[nowelement];
        if (TEMPERA[nowelement] == '\0')
          {
            memcpy_P(buffer, CH + 7*0, 7);
            m.writeSprite(32, 0, buffer);
          }
        else if (TEMPERA[nowelement] >= 32) 
        {
          TEMPERA[nowelement] -= 32;
          memcpy_P(buffer, CH + 7*TEMPERA[nowelement], 7);
          m.writeSprite(32, 0, buffer);
          m.setColumn(32 + buffer[0], 0);
         
          nowelement++;
          nowbuf = 0;
        }
        else
        {
          nowelement++;
        }
      }
      
    }                                                 //溫度顯示結束
    else                                              // 左移
    {
      if (currentMillis - leftpreviousMillis > 100)
        {
         // delay(100);
          memcpy_P(buffer, CH + 7*0, 7);
          m.writeSprite(32, 0, buffer);
          m.shiftLeft(false, false);
        
          leftpreviousMillis = millis();
        }
    }
  }
}
