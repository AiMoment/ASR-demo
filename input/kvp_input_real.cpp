#include "kvp_input_real.h"

#include <QDebug>
#include <QAudioInput>
#include <QAudioDeviceInfo>

KVP_Input_Real::KVP_Input_Real()
{

}

void KVP_Input_Real::initRealMic()
{
}

void KVP_Input_Real::startMic()
{
    QAudioDeviceInfo audioDevice = QAudioDeviceInfo::defaultInputDevice();
    qDebug() << "default device name : " << audioDevice.deviceName();

    formatSound = audioDevice.nearestFormat(formatSound);
    formatSound.setSampleRate(16000);
    audioInputSound = new QAudioInput(audioDevice, formatSound, this);

    inputIODevice = audioInputSound->start();
    if (!inputIODevice) {
        qDebug() << "input sound start error. why:" << audioInputSound->error();
        stopMic();
        return;
    }
    connect(inputIODevice, &QIODevice::readyRead, this, &KVP_Input_Real::readData);
}

void KVP_Input_Real::stopMic()
{
    audioInputSound->stop();
    inputIODevice->deleteLater();
    audioInputSound->deleteLater();
}

int KVP_Input_Real::open(QString &device)
{

}

void KVP_Input_Real::readData()
{
    //如果输入为空则返回空
    if (!audioInputSound) {
        qDebug() << "输入为空!";
        return;
    }

    //处理音频输入，recordBuffer中保存音频数据，len是获取到的数据的长度
    QByteArray recordBuffer(BufferSize, 0);
    int len = audioInputSound->bytesReady();
    if (len > 4096) {
        len = 4096;
    }

    // 真正进行将录音数据保存到recordBuffer，每次传输的pcm数据就是recordBuffer
    qint64 l = inputIODevice->read(recordBuffer.data(), len);
    if (l <= 0) {
        return;
    }

    emit packetReady(recordBuffer, l);
}
