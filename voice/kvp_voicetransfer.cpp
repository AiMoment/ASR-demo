#include "kvp_voicetransfer.h"

#include <QDebug>

#include "ai/aiengine.h"
#include "input/kvp_input_real.h"

KVP_VoiceTransfer::KVP_VoiceTransfer(QObject *parent) : QObject(parent)
{
    enableTrans = false;
    initInput();
    initAIEngine();
}

void KVP_VoiceTransfer::testStart()
{
    qDebug() << "testStart";

    enableTrans = true;
    aiEngine->enable();

    qDebug() << "before startMic";
    inputReal->startMic();
}

void KVP_VoiceTransfer::testEnd()
{
    enableTrans = false;
    dataLen = 0;
    memset(audioData, 0x00, MAX_DATASIZE);
    aiEngine->disable();
}

void KVP_VoiceTransfer::initAIEngine()
{
    if (!aiEngine) {
        aiEngine = new AIEngine;
    }

    connect(aiEngine, &AIEngine::parsetextMessageResultSignal, this, &KVP_VoiceTransfer::handleAItranslateData);
}

void KVP_VoiceTransfer::initInput()
{
    inputReal = new KVP_Input_Real;
    connect(inputReal, &KVP_Input_Real::audioDataPackReadySignal, this, &KVP_VoiceTransfer::handleDataInput);
}

void KVP_VoiceTransfer::startReal()
{
    inputReal->startMic();
}

void KVP_VoiceTransfer::handleDataInput(QByteArray data, int len)
{
    if (!enableTrans)
        return;
    // 获取到的音频数据进行拼接以满足 ai 接口需求
     if (dataLen + len >= MAX_DATASIZE) {
         aiEngine->feedData(audioData, dataLen);
         dataLen = 0;
         memset(audioData, 0x00, MAX_DATASIZE);
     }
     else {
         memcpy(audioData + dataLen, data.data(), len);
         dataLen += len;
     }
}

void KVP_VoiceTransfer::handleAItranslateData(QString trans)
{
    emit translateOk(trans);
    qDebug() << "get ai translate data : " << trans;
}
