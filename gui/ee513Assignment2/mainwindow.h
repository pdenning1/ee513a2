#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include "MQTTClient.h"

#define ADDRESS     "tcp://127.0.0.1:1883"
#define CLIENTID    "rpi2"
#define AUTHMETHOD  "pdenn"
#define AUTHTOKEN   "123456"
#define TOPIC       "ee513/Roll"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L


#define TOPIC_TEMP      "ee513/CPUTemp"
#define TOPIC_TIME	"ee513/Time"
#define TOPIC_ACCLX      "ee513/Accl/X"
#define TOPIC_ACCLY      "ee513/Accl/Y"
#define TOPIC_ACCLZ      "ee513/Accl/Z"
#define TOPIC_ROLL      "ee513/Roll/Angle"
#define TOPIC_PITCH     "ee513/Pitch/Angle"

#define TOPIC_ANGLE "ee513/+/Angle"
#define TOPIC_ACCL "ee513/Accl/#"

#define TOPIC_TEST    "ee513/Test"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_MQTTmessage(QString message);

signals:
    void messageSignal(QString message);

private:
    Ui::MainWindow *ui;
    void update(int graph);
    int time;
    float newData;
    float accl[3];
    float angl[2];
    double connectedTime;
    QString currentTopic;
    MQTTClient client;
    volatile MQTTClient_deliveryToken deliveredtoken;

    int parseJSONData(QString str);
    QString getAxisLabel(QString topic);
    void setupGraphs();

    friend void delivered(void *context, MQTTClient_deliveryToken dt);
    friend int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    friend void connlost(void *context, char *cause);
};

void delivered(void *context, MQTTClient_deliveryToken dt);
int  msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);

#endif // MAINWINDOW_H

