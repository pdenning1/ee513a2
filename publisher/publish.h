// Based on the Paho C code example from www.eclipse.orgpaho
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <time.h>
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
#define TOPIC_TIME	"ee513/Time"
#define TOPIC_ACCLX      "ee513/AcclX"
#define TOPIC_ACCLY      "ee513/AcclY"
#define TOPIC_ACCLZ      "ee513/AcclZ"
#define TOPIC_ROLL      "ee513/Roll"
#define TOPIC_PITCH     "ee513/Pitch"

namespace ee513a2{

class Publish{
private:
   MQTTClient client;
   MQTTClient_connectOptions conn_opts;
   MQTTClient_willOptions will_opts;
   MQTTClient_message pubmsg;
   MQTTClient_deliveryToken token;
   ADXL345* theAdxl;

   char str_payload[100];

    float getCPUTemperature();
    void getTime(char *theTime);
    void generateJson(char* str_payload, const char* topic, char* data);
    int publishMessage(char* str_payload, const char* topic);

public:
    Publish();
    ~Publish();
    void disconnect();    
    void publishAll();
};
}
