/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijincheng <baijincheng@kylinos.cn>
 */
#ifndef ASRTHREAD_H
#define ASRTHREAD_H

#include <QWidget>
#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QThread>

#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QGSettings>

#include <QMultimedia>
#include <QtMultimedia>
#include <QAudio>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QIODevice>
#include <QFile>
#include <QFileSystemWatcher>


#define BufferSize 35280 // 读取的录音数据长度

struct WAVFILEHEADER
{
    char RiffName[4];    // RIFF 头(标志)
    uint32_t RiffLength; // 从下一个开始到文件结尾的字节数
    char WavName[4];     // WAVE 文件
    char FmtName[4];     // fmt 波形格式

    uint32_t FmtLength;      // 数据格式
    uint16_t AudioFormat;    // 音频格式是否是压缩的PCM编码，1无压缩，大于1有压缩
    uint16_t ChannleNumber;  // 声道数量
    uint32_t SampleRate;     // 采样频率
    uint32_t BytesPerSecond; // byterate(每秒字节数) = 采样率(Hz)*样本数据位数(bit)*声道数/8
    uint16_t BytesPerSample; // 块对齐/帧大小 framesize = 通道数 * 数据位数
    uint16_t BitsPerSample;  // 样本数据位数

    char DATANAME[4];    // catheFileName , QString wavF数据块中的块头
    uint32_t DataLength; // 实际音频数据的大小
};

class AsrThread : public QObject
{
    Q_OBJECT
public:
    explicit AsrThread(QObject *parent = nullptr);

    int soundVolume = 65; // 初始音量为70

    QString fileName;
    QString default_Location;
    QString endFileName;
    QString absolutionPath; // raw中间文件 存放的绝对路径, raw 和pcm 是一样存储的
    QString outputFileName; // 输出的文件名
    QString recordTime; // 输出的录音时间
    QString rawFileName; // raw 临时文件名

    QGSettings *recordGSettings = nullptr; // 参考录音的gsettings，这里不额外处理
    bool isRecordStart = false; // 录音是否开始
    QString getRecordPathByGSettings(); // gsettings得到录音路径
    void setRecordPathByGsettings(QString filePath); //更新配置文件

    qint64 toConvertPCM(QString catheFileName, QString pcmFileName);
    qint64 toConvertWAV(QString catheFileName, QString wavFileName);
    qint64 toConvertMp3(QString catheFileName, QString mp3FileName);


private:
    QAudioFormat Mp3();
    QAudioFormat Wav();
    QAudioFormat M4a();
    QAudioFormat format;

    QIODevice *inputIODevice = nullptr;

    QAudioInput *audioInputSound = nullptr; // 负责监听声音
    QAudioInput *audioInputFile = nullptr;   // 将音频输入文件

    QFile *file = nullptr;
    short *outdata = nullptr; // outdata 为转换后的录音数据
    qint64 len; // len是获取到的数据的长度
    QAudioFormat formatSound;

    QAudioOutput *audioOutputSound = nullptr;
    QAudioDeviceInfo audioDevice;

    int value; // 每次检测到的声音振幅值
    int MaxValue; // 每次检测到的声音振幅值最大值

    int useVolumeSample(qreal iSample); // 求每次检测到的振幅组中最大值

    QAudioDeviceInfo monitorVoiceSource(int sourceType); // 当前录音设备来源
    QString setDefaultPath(QString path); // 设置默认存储路径解决中英文路径冲突问题
    QString pathSetting(); // 录音文件存储位置

    QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);
    qreal getPeakValue(const QAudioFormat &format); // 获取录制声音的最大采样值
    template <class T> QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);


public slots:
    void initMonitor();
    void readyReadRecordData(); // 读取录音数据
    void startRecord();  // 开始录制音频
    void stopRecord(); // 停止保存录音
    void playRecord();
    void pauseRecord(); // 暫停录制

};

#endif // ASRTHREAD_H
