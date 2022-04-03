#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

#include <json-c/json.h>
#include <wiringPi.h>
#include <pthread.h>


#define ADDRESS     "tcp://192.168.1.14:1883"
#define CLIENTID    "rpi2"
#define AUTHMETHOD  "pdenn"
#define AUTHTOKEN   "123456"
#define TOPIC       "ee513/CPUTemp"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

#define ALARM_TEMP 40 // define the temperature at which the led should flash

#define TOPIC_TEMP      "ee513/CPUTemp"
#define TOPIC_TIME	"ee513/Time"
#define TOPIC_ACCLX      "ee513/Accl/X"
#define TOPIC_ACCLY      "ee513/Accl/Y"
#define TOPIC_ACCLZ      "ee513/Accl/Z"
#define TOPIC_ROLL      "ee513/Roll/Angle"
#define TOPIC_PITCH     "ee513/Pitch/Angle"

#define LED_PIN 	0     // wiringPi pin 0 is GPIO17  http://wiringpi.com/pins/

volatile MQTTClient_deliveryToken deliveredtoken;


// thread function to control flashing of LED. based on code on page 239 of Exploring Raspberry Pi
void *threadFunction(void *value) {
    int *x = (int *)value;
    if(*x = 1){
        digitalWrite(LED_PIN, HIGH);
        sleep(2);
        digitalWrite(LED_PIN, LOW);
        sleep(1);
    }
    else {
        digitalWrite(LED_PIN, LOW);
    }
    return x;
}

void parseJson(json_object * jobj, int* alarm) {
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
    type = json_object_get_type(val);
    switch (type) {
        case json_type_double: printf("type: json_type_double, ");
        printf("value: %f\n", json_object_get_double(val));
        if(json_object_get_double(val) > ALARM_TEMP){
            printf("TEMPERATURE is greater than %d\n", ALARM_TEMP);
            *alarm = 1;
        }
        else {
            *alarm = 0;
        }
        break;
    }
    }
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadptr;
    //char* str_payload = (char*)context; // we save the message into this string
    int* alarm = (int*)context;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = (char*) message->payload;

    int i;
    for(i=0; i<message->payloadlen; i++) {
        //str_payload[i] = *payloadptr;
        putchar(*payloadptr++);
    }
    //str_payload[i] = 0; // string needs to be null terminated

    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    json_object * jobj = json_tokener_parse(str_payload);
    parseJson(jobj, alarm);

    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

//void onEnter(void* ch) {
//    int* chr = (int*)ch;
//    while(true) {
//        *chr = getchar();
//    }
//}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
    
    
    int* alarm;
    *alarm = 0;

    pthread_t = thread;
    if(pthread_create(&thread, NULL, &threadFunction, alarm)!=0){
        cout << "Failed to create the thread" << endl;
        return 1;
    }


    wiringPiSetup(); // this sets up wiring pi with pin numbers according to wiringPi layout
    pinMode(LED_PIN, OUTPUT);

    float sensorTemp = 0;
    //char str_payload[100]; // same max length as publisher payload 

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 0;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, alarm, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);


    do {

        ch = getchar();
	    //puts(str_payload);

    } while(ch!='Q' && ch != 'q');

    pthread_join(thread, NULL); // end LED lighting thread

    digitalWrite(LED_PIN, LOW); // turn off led before closing

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
