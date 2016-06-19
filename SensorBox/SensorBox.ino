#include <dht.h>
#include <YunClient.h>
#include <PubSubClient.h>
#include "config.h"

#define CLIENT_ID "sensor_box"

void subscribeChannel();
void callback(char* topic, byte* payload, unsigned int length);
void printReceiveData(char* topic, byte* payload, unsigned int length);
void infraredTransmitter(byte* payload);

YunClient yun;
PubSubClient client(yun);

void setup() {
    Serial.begin(9600);
    Bridge.begin();
    
    client.setServer(server, 1883);
    client.setCallback(callback);
    
    // Allow the hardware to sort itself out
    delay(1500);
}

long currentMillis,previousMillis,interval=5000;
dht DHT;

int FIREvalue;
int GASvalue;
int COvalue;
String data;
  
void loop() {
    
    currentMillis = millis();
       
    if (client.connected()) {
        if(currentMillis - previousMillis > interval) {
            previousMillis = currentMillis;

            DHT.read11(dht_dpin);
            String s_Temp = "", s_Hum = "", s_Fire = "", s_Gas = "", s_CO = "";
            s_Hum += DHT.humidity;
            s_Temp += DHT.temperature;
            
            //FIREvalue = analogRead(FIREpin);
            s_Fire += analogRead(FIREpin);
            //GASvalue = analogRead(GASpin);
            s_Gas += analogRead(GASpin);
            //COvalue = analogRead(COpin);
            s_CO += analogRead(COpin);
                                    
            client.publish("sensor_temperature", s_Temp.c_str());
            client.publish("sensor_humidity", s_Hum.c_str());
            client.publish("sensor_gas", s_Gas.c_str());
            client.publish("sensor_co", s_CO.c_str());
            if((int)analogRead(FIREpin)<400){
              client.publish("fire", "enable");
              //debug
              //Serial.print("\nfire enable ");
              }
            else{
              client.publish("fire", "disable");
              //debug
              //Serial.print("\nfire disable ");
            }
            
            // debug
            Serial.print("\n\n\nTemp = ");
            Serial.print(s_Temp);
            Serial.print("\nHum = ");
            Serial.print(s_Hum);
            Serial.print("\nFire = ");
            Serial.print(s_Fire);
            Serial.print("\nGAS = ");
            Serial.print(s_Hum);
            Serial.print("\nCO = ");
            Serial.print(s_CO);
        }
    }
    else {
        reconnect();
    }
    client.loop();
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(CLIENT_ID)) {
            Serial.println("connected");
            // resubscribe
            subscribeChannel();
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void subscribeChannel() {
    client.subscribe("clients_status");
}

void callback(char* topic, byte* payload, unsigned int length) {
    //debug
    printReceiveData(topic, payload, length);

    if (strcpy(topic, "clients_status") && length == 6 && memcpy(payload, "status", 6)) {
        client.publish("clients_status", CLIENT_ID);
    }
}

void printReceiveData(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i=0;i<length;i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

