#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include <QueueList.h>

#include <IRremote.h>
#include "saiiler_ir_code.h"
IRsend irsend;

SoftwareSerial debugPort(6, 7); // RX, TX
ESP esp(&debugPort, &Serial, 4);
MQTT mqtt(&esp);
boolean wifiConnected = false;

QueueList<String> queue_command;

void wifiCb(void* response) {
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
    Serial.println("Connected");
    mqtt.subscribe("fan_ir_command");
}

void mqttDisconnected(void* response) {
}

void mqttData(void* response) {
    RESPONSE res(response);

    Serial.print("Received: topic=");
    String topic = res.popString();
    Serial.println(topic);

    Serial.print("data=");
    String data = res.popString();
    Serial.println(data);

    if (topic == "fan_ir_command") {
        queue_command.push(data);
    }
}
void mqttPublished(void* response) {
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
        while (!queue_command.isEmpty()) {
            String data = queue_command.pop();
            if (data == "steering") {
                send__fan_ir_raw_data(raw_data_steering);
            }
            else if (data == "timing") {
                send__fan_ir_raw_data(raw_data_timing);
            }
            else if (data == "close") {
                send__fan_ir_raw_data(raw_data_close);
            }
            else if (data == "super") {
                send__fan_ir_raw_data(raw_data_super);
            }
            else if (data == "gale") {
                send__fan_ir_raw_data(raw_data_gale);
            }
            else if (data == "breeze") {
                send__fan_ir_raw_data(raw_data_breeze);
            }
        }
    }
}

void send__fan_ir_raw_data(const unsigned int raw_data_p[]) {
    unsigned int buf[RAW_DATA_LEN];
    memcpy_P(buf, raw_data_p, RAW_DATA_LEN * sizeof(unsigned int));
    irsend.sendRaw(buf, RAW_DATA_LEN, KHZ);
}

