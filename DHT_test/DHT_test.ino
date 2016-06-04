#include <dht.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define dht_dpin 3
dht DHT;

#define LED0_PIN 5

long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 5000;

byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 150);
IPAddress server(192, 168, 0, 7);

void printReceiveData(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    }
    Serial.println();
}

void callback(char* topic, byte* payload, unsigned int length) {
    //debug
    printReceiveData(topic, payload, length);

    if (strcmp(topic, "LED0") == 0) {
        Serial.println("topic is LED0");
        if (memcmp(payload, "on", 2) == 0) {
            Serial.println("payload is on");
            digitalWrite(LED0_PIN, HIGH);
        }
        else if (memcmp(payload, "off", 3) == 0) {
            Serial.println("payload is off");
            digitalWrite(LED0_PIN, LOW);
        }
    }
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // resubscribe
      client.subscribe("LED0");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
    Serial.begin(9600);
    
    client.setServer(server, 1883);
    client.setCallback(callback);
    
    Ethernet.begin(mac, ip);
    // Allow the hardware to sort itself out
    delay(1500);

    pinMode(LED0_PIN, OUTPUT);
    digitalWrite(LED0_PIN, LOW);
}

void loop() {
    currentMillis = millis();
       
    if (client.connected()) {
        if(currentMillis - previousMillis > interval) {
            previousMillis = currentMillis;

            DHT.read11(dht_dpin);
            String s = "";
            s += DHT.humidity;
            s += ",";
            s += DHT.temperature;
            
            client.publish("sensor", s.c_str());
            // debug
            Serial.println(s);
        }
    }
    else {
        reconnect();
    }
    client.loop();
}
