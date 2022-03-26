// Based on the Paho C code example from www.eclipse.orgpaho
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <MQTTClient.h>
#include "ADXL345.h"
#define  CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
using namespace std;
using namespace exploringRPi;

//Please replace the following address with the address of your server
#define ADDRESS    "tcp://192.168.1.14:1883"
#define CLIENTID   "rpi1"
#define AUTHMETHOD "pdenn"
#define AUTHTOKEN  "123456"
#define TOPIC      "ee513/CPUTemp"
#define QOS        1
#define TIMEOUT    10000L

#define TOPIC_TEMP      "ee513/CPUTemp"
#define TOPIC_ACCL "ee513/Accl"
#define TOPIC_ROLL "ee513/Roll"
#define TOPIC_PITCH "ee513/Pitch"


float getCPUTemperature() {         //get the CPU temperature
   int cpuTemp;                     //store as an int
   fstream fs;
   fs.open(CPU_TEMP, fstream::in);  //read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}

// helper function to generate a json payload from topic string and data string
// data must first be converted to string form
void generateJson(char* str_payload, const char* topic, char* data) {
   sprintf(str_payload, "{\"d\":{\"%s\": %s }}", topic, data);
}

int publishMessage(char* str_payload, const char* topic){
   // setup client and connect to broker
   MQTTClient client;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   MQTTClient_deliveryToken token;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   opts.keepAliveInterval = 20;
   opts.cleansession = 1;
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }

   // create and send the message, wait for completion
   pubmsg.payload = str_payload;
   pubmsg.payloadlen = strlen(str_payload);
   pubmsg.qos = QOS;
   pubmsg.retained = 0;
   MQTTClient_publishMessage(client, topic, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload <<
        " \non topic " << topic << " for ClientID: " << CLIENTID << endl;
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered." << endl;

   // disconnect and destoy client
   MQTTClient_disconnect(client, 10000);
   MQTTClient_destroy(&client);
   return rc;
}


int main(int argc, char* argv[]) {
   char str_payload[100];           //Set your max message size here
   // MQTTClient client;
   // MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   // MQTTClient_message pubmsg = MQTTClient_message_initializer;
   // MQTTClient_deliveryToken token;
   // MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   // opts.keepAliveInterval = 20;
   // opts.cleansession = 1;
   // opts.username = AUTHMETHOD;
   // opts.password = AUTHTOKEN;
   // int rc;
   // if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
   //    cout << "Failed to connect, return code " << rc << endl;
   //    return -1;
   // }

   // Code added to interface to ADXL345
   ADXL345 theAdxl(1, 0x53);
   theAdxl.setResolution(ADXL345::NORMAL);
   theAdxl.setRange(ADXL345::PLUSMINUS_4_G);
   theAdxl.readSensorState();
   
   char roll[20];
   sprintf(roll, "%f", theAdxl.getRoll());
   generateJson(str_payload, TOPIC_ROLL, roll);
   publishMessage(str_payload, TOPIC_ROLL);

   char temp[20];
   sprintf(temp, "%f", getCPUTemperature());
   generateJson(str_payload, TOPIC_TEMP, temp);
   publishMessage(str_payload, TOPIC_TEMP);
   

   // // create the payload
   // sprintf(str_payload, "{\"d\":{\"CPUTemp\": %f, \"X Accel\": %d, \"Y Accel\": %d, \"Z Accel\": %d, \"Pitch\": %f, \"Roll\": %f }}",
	// 		 getCPUTemperature(), theAdxl.getAccelerationX(), theAdxl.getAccelerationY(), theAdxl.getAccelerationZ(),
	// 		theAdxl.getPitch(), theAdxl.getRoll());
   
   // // create and send the message, wait for completion
   // pubmsg.payload = str_payload;
   // pubmsg.payloadlen = strlen(str_payload);
   // pubmsg.qos = QOS;
   // pubmsg.retained = 0;
   // MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
   // cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
   //      " seconds for publication of " << str_payload <<
   //      " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;
   // rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   // cout << "Message with token " << (int)token << " delivered." << endl;

   // // disconnect and destoy client
   // MQTTClient_disconnect(client, 10000);
   // MQTTClient_destroy(&client);
   // return rc;
}
