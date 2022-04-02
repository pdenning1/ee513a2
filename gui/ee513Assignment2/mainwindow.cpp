#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>

MainWindow *handle;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->time = 0;
    this->setWindowTitle("EE513 Assignment 2");

    this->setupGraphs();

    this->ui->customPlot->yAxis->setLabel("");
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    this->ui->customPlot->xAxis->setTicker(timeTicker);
    this->ui->customPlot->yAxis->setRange(-180,180);
    this->ui->customPlot->replot();
    QObject::connect(this, SIGNAL(messageSignal(QString)),
                     this, SLOT(on_MQTTmessage(QString)));
    ::handle = this;

    // ad the topics to the combo box
    this->ui->comboBoxTopic->addItem(TOPIC_TEMP);
    this->ui->comboBoxTopic->addItem(TOPIC_ACCLX);
    this->ui->comboBoxTopic->addItem(TOPIC_ACCLY);
    this->ui->comboBoxTopic->addItem(TOPIC_ACCLZ);
    this->ui->comboBoxTopic->addItem(TOPIC_PITCH);
    this->ui->comboBoxTopic->addItem(TOPIC_ROLL);
    this->ui->comboBoxTopic->addItem(TOPIC_ACCL);
    this->ui->comboBoxTopic->addItem(TOPIC_ANGLE);

    this->currentTopic = "";
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getAxisLabel(QString topic){
    QString axisLabel = "";
    if(topic.compare(TOPIC_TEMP) == 0){
        axisLabel = "Temperature (C)";
    }
    else if(topic.compare(TOPIC_ACCLX) == 0){
        axisLabel = "Acceleration-X";
    }
    else if(topic.compare(TOPIC_ACCLY) == 0){
        axisLabel = "Acceleration-Y ";
    }
    else if(topic.compare(TOPIC_ACCLZ) == 0){
        axisLabel = "Acceleration-Z";
    }
    else if(topic.compare(TOPIC_PITCH) == 0){
        axisLabel = "Pitch (degrees)";
    }
    else if(topic.compare(TOPIC_ROLL) == 0){
        axisLabel = "Roll (degrees)";
    }
    else if(topic.compare(TOPIC_ACCL) == 0){
        axisLabel = "Acceleration-any direction";
    }
    else if(topic.compare(TOPIC_ANGLE) == 0){
        axisLabel = "Roll angle/Pitch angle (degrees)";
    }

    return axisLabel;
}

// Helper function to set up the 3 qcustomplot graphs
void MainWindow::setupGraphs(){
    this->ui->customPlot->addGraph();
    this->ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
    this->ui->customPlot->addGraph();
    this->ui->customPlot->graph(1)->setPen(QPen(Qt::red));
    this->ui->customPlot->addGraph();
    this->ui->customPlot->graph(2)->setPen(QPen(Qt::green));
}


// Parses the json object. Returns an int indicating which graph# to update
int MainWindow::parseJSONData(QString str){
    int graph = 0;

    QJsonParseError *jsonError = new QJsonParseError();
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), jsonError);
    QJsonObject obj = doc.object();

    // we need to handle it differently if we are using a wildcard (TOPIC_ACCL or TOPIC_ANGLE)
    // as this involves multiple plots
    if(currentTopic.compare(TOPIC_ACCL) == 0){
        if(obj.keys().at(0).compare(TOPIC_ACCLX) == 0){
            this->newData = (float) obj[TOPIC_ACCLX].toDouble();
            graph = 0;
        }
        if(obj.keys().at(0).compare(TOPIC_ACCLY) == 0){
            this->newData = (float) obj[TOPIC_ACCLY].toDouble();
            graph = 1;
        }
        if(obj.keys().at(0).compare(TOPIC_ACCLZ) == 0){
            this->newData = (float) obj[TOPIC_ACCLZ].toDouble();
            graph = 2;
        }
    }
    else if(currentTopic.compare(TOPIC_ANGLE) == 0){
        if(obj.keys().at(0).compare(TOPIC_ROLL) == 0){
            this->newData = (float) obj[TOPIC_ROLL].toDouble();
            graph = 0;
        }
        if(obj.keys().at(0).compare(TOPIC_PITCH) == 0){
            this->newData = (float) obj[TOPIC_PITCH].toDouble();
            graph = 1;
        }
    }
    else {
        this->newData = (float) obj[currentTopic].toDouble();
    }
    qDebug() << "Value received " << newData;

    qDebug() << jsonError->errorString();
    delete(jsonError);

    return graph;
}

void MainWindow::update(int graph){
    // For more help on real-time plots, see: http://www.qcustomplot.com/index.php/demos/realtimedatademo
    static QTime time(QTime::currentTime());
    double key = (time.elapsed()/1000.0) - connectedTime; // time elapsed since start of demo, in seconds
    ui->customPlot->graph(graph)->addData(key,newData);
    ui->customPlot->graph(graph)->rescaleKeyAxis(true);
    ui->customPlot->replot();
    QString text = QString("Value added is %1").arg(this->newData);
    ui->outputEdit->setText(text);
}


void MainWindow::on_connectButton_clicked()
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;

    // next 3 lines saves the currently selected topic in the combobox to a const char* to be passed in MQTTClient_subscribe
    currentTopic = this->ui->comboBoxTopic->currentText();
    QByteArray ba = currentTopic.toLocal8Bit();
    const char *topic = ba.data();

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)==0){
        ui->outputText->appendPlainText(QString("Callbacks set correctly"));
    }
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        ui->outputText->appendPlainText(QString("Failed to connect, return code %1").arg(rc));
    }
    ui->outputText->appendPlainText(QString("Subscribing to topic ") + currentTopic + QString( " for client " CLIENTID));
    //ui->outputText->appendPlainText()
    int x = MQTTClient_subscribe(client, topic, QOS);
    ui->outputText->appendPlainText(QString("Result of subscribe is %1 (0=success)").arg(x));

    // set the graph axis label to match the current topic
    this->ui->customPlot->yAxis->setLabel(getAxisLabel(currentTopic));
    this->ui->customPlot->clearGraphs();
    this->setupGraphs();
    this->ui->customPlot->replot();

    ui->outputEdit->setText("Waiting for message from publisher...");

    // reset connectedtime so graph plotting starts from zero
    static QTime time(QTime::currentTime());
    this->connectedTime = time.elapsed()/1000.0;
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    (void)context;
    // Please don't modify the Window UI from here
    qDebug() << "Message delivery confirmed";
    handle->deliveredtoken = dt;
}

/* This is a callback function and is essentially another thread. Do not modify the
 * main window UI from here as it will cause problems. Please see the Slot method that
 * is directly below this function. To ensure that this method is thread safe I had to
 * get it to emit a signal which is received by the slot method below */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    (void)context; (void)topicLen;
    qDebug() << "Message arrived (topic is " << topicName << ")";
    qDebug() << "Message payload length is " << message->payloadlen;
    QString payload;
    payload.sprintf("%s", (char *) message->payload).truncate(message->payloadlen);
    emit handle->messageSignal(payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/** This is the slot method. Do all of your message received work here. It is also safe
 * to call other methods on the object from this point in the code */
void MainWindow::on_MQTTmessage(QString payload){
    ui->outputText->appendPlainText(payload);
    ui->outputText->ensureCursorVisible();

    //ADD YOUR CODE HERE
    int graph = parseJSONData(payload);
    this->update(graph);
}

void connlost(void *context, char *cause) {
    (void)context; (void)*cause;
    // Please don't modify the Window UI from here
    qDebug() << "Connection Lost" << endl;
}

void MainWindow::on_disconnectButton_clicked()
{
    qDebug() << "Disconnecting from the broker" << endl;
    MQTTClient_disconnect(client, 10000);
    //MQTTClient_destroy(&client);
}
