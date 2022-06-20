#ifndef KVP_INPUT_REAL_H
#define KVP_INPUT_REAL_H

#include "kvp_input.h"

#include <QAudioFormat>

#define BufferSize 35280

class KVP_Packet;

class QByteArray;
class QIODevice;
class QAudioInput;
class QAudioFormat;
class QAudioDeviceInfo;

class KVP_Input_Real : public KVP_Input
{
    Q_OBJECT
public:
    KVP_Input_Real();

    // 初始化 mic 输入
    void initRealMic();
    void startMic();
    void stopMic();
    int open(QString& device);

signals:
    void packetReady(QByteArray, int);

private slots:
    void readData();

private:
    QIODevice *inputIODevice = nullptr;
    QAudioInput *audioInputSound = nullptr; // 负责监听声音
    QAudioFormat formatSound;
    QByteArray audioData;
};

#endif // KVP_INPUT_REAL_H
