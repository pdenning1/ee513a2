// // Based on the Paho C code example from www.eclipse.orgpaho

#include "publish.h"

namespace ee513a2{

Publish::Publish(){
   // setup client and connect to broker
   this->conn_opts = MQTTClient_connectOptions_initializer;
   this->will_opts = MQTTClient_willOptions_initializer; // adding LWT
   this->pubmsg = MQTTClient_message_initializer;
   //MQTTClient_deliveryToken token;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   conn_opts.keepAliveInterval = 20;
   conn_opts.cleansession = 1;
   conn_opts.username = AUTHMETHOD;
   conn_opts.password = AUTHTOKEN;
   conn_opts.will = &will_opts;
   will_opts.topicName = TOPIC_TEMP;
   will_opts.message = "{\"ee513/CPUTemp\": \"Client rpi1 lost connection\" }";
   will_opts.qos = QOS;
   int rc;
   if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return;
   }

   // setup the ADXL345
   theAdxl = new ADXL345(1, 0x53);
   theAdxl->setResolution(ADXL345::NORMAL);
   theAdxl->setRange(ADXL345::PLUSMINUS_4_G);
}

Publish::~Publish(){
   cout << "Disconnecting client..." << endl;
   MQTTClient_disconnect(client, 10000);
   MQTTClient_destroy(&client);
}


float Publish::getCPUTemperature() {         //get the CPU temperature
   int cpuTemp;                     //store as an int
   fstream fs;
   fs.open(CPU_TEMP, fstream::in);  //read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}


// get the current time and save it to a string. Source: http://www.cplusplus.com/reference/ctime/localtime/
void Publish::getTime(char *theTime){
   time_t rawTime;
   struct tm *timeInfo;

   time(&rawTime);
   timeInfo = localtime(&rawTime);
   sprintf(theTime, "%s", asctime(timeInfo));
   theTime[24] = 0; // the string has a newline char here [24], so change to null
}

// helper function to generate a json payload from topic string and data string
// data must first be converted to string form
void Publish::generateJson(char* str_payload, const char* topic, char* data) {
    sprintf(str_payload, "{\"%s\": %s }", topic, data);
}

int Publish::publishMessage(char* str_payload, const char* topic){
   // create and send the message, wait for completion
   int rc;
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

   return rc;
}

void Publish::disconnect(){
   cout << "Disconnecting client..." << endl;
   MQTTClient_disconnect(client, 10000);
   MQTTClient_destroy(&client);
}

void Publish::publishAll(){
   theAdxl->readSensorState();

   //for each topic, save data as string, generate json, and the publish message
   char acclX[20];
   sprintf(acclX, "%d", theAdxl->getAccelerationX());
   generateJson(str_payload, TOPIC_ACCLX, acclX);
   publishMessage(str_payload, TOPIC_ACCLX);

   char acclY[20];
   sprintf(acclY, "%d", theAdxl->getAccelerationY());
   generateJson(str_payload, TOPIC_ACCLY, acclY);
   publishMessage(str_payload, TOPIC_ACCLY);

   char acclZ[20];
   sprintf(acclZ, "%d", theAdxl->getAccelerationZ());
   generateJson(str_payload, TOPIC_ACCLZ, acclZ);
   publishMessage(str_payload, TOPIC_ACCLZ);
   
   char roll[20];
   sprintf(roll, "%f", theAdxl->getRoll());
   generateJson(str_payload, TOPIC_ROLL, roll);
   publishMessage(str_payload, TOPIC_ROLL);

   char pitch[20];
   sprintf(pitch, "%f", theAdxl->getPitch());
   generateJson(str_payload, TOPIC_PITCH, pitch);
   publishMessage(str_payload, TOPIC_PITCH);

   char temp[20];
   sprintf(temp, "%f", getCPUTemperature());
   generateJson(str_payload, TOPIC_TEMP, temp);
   publishMessage(str_payload, TOPIC_TEMP);

   char theTime[30];
   getTime(theTime);
   generateJson(str_payload, TOPIC_TIME, theTime);
   publishMessage(str_payload, TOPIC_TIME);
}

} // namesapce ee513a2
