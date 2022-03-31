#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include <json-c/json.h>

#define ADDRESS     "tcp://192.168.1.14:1883"
#define CLIENTID    "rpi2"
#define AUTHMETHOD  "pdenn"
#define AUTHTOKEN   "123456"
#define TOPIC       "ee513/CPUTemp"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

#define TOPIC_TEMP      "ee513/CPUTemp"
#define TOPIC_TIME	"ee513/Time"
#define TOPIC_ACCLX      "ee513/AcclX"
#define TOPIC_ACCLY      "ee513/AcclY"
#define TOPIC_ACCLZ      "ee513/AcclZ"
#define TOPIC_ROLL      "ee513/Roll"
#define TOPIC_PITCH     "ee513/Pitch"

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    char* str_payload = (char*)context;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = (char*) message->payload;
    for(i=0; i<message->payloadlen; i++) {
        str_payload[i] = *payloadptr;
        putchar(*payloadptr++);
    }
    str_payload[i] = 0;
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void parseJson(json_object * jobj) {
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
    type = json_object_get_type(val);
    switch (type) {
        case json_type_double: printf("type: json_type_double, ");
        printf("value: %f", json_object_get_double(val));
        if(json_object_get_double(val) > 40){
            printf("CPU temp is greater than 40");
        }
        break;
    }
    }
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    float sensorTemp = 0;
    char str_payload[100]; // same max length as publisher payload 

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, str_payload, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    //char* string = "{ \"PI\" : 3.140000 }";
    //printf("JSON string is: %s\n", string);
    //json_object * jobj = json_tokener_parse(string);
    //parseJson(jobj);

    do {
        ch = getchar();
	puts(str_payload);
        printf("CPUTemp is: ");
        json_object * jobj = json_tokener_parse(str_payload);
        parseJson(jobj);
        printf("\n");
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
