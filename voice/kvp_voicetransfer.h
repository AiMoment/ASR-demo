#ifndef KVP_VOICETRANSFER_H
#define KVP_VOICETRANSFER_H

#include <QObject>

#define MAX_DATASIZE 8192

class AIEngine;
class KVP_Input_Real;

class KVP_VoiceTransfer : public QObject
{
    Q_OBJECT
public:
    explicit KVP_VoiceTransfer(QObject *parent = nullptr);

    void testStart();
    void testEnd();

    void initAIEngine();
    void initInput();

    void startReal();
signals:
    void translateOk(QString);

private slots:
    void handleDataInput(QByteArray data, int len);
    void handleAItranslateData(QString trans);

private:
    bool enableTrans;
    int dataLen = 0;
    char audioData[MAX_DATASIZE];
    AIEngine *aiEngine = nullptr;
    KVP_Input_Real *inputReal = nullptr;

    void sendData2AI();
};

#endif // KVP_VOICETRANSFER_H
