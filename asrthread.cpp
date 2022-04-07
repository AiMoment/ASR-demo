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
#include "asrthread.h"
#include "mainwindow.h"
#include "fileoperation.h"
#include <unistd.h>

#include <ukui-log4qt.h>

AsrThread::AsrThread(QObject *parent) : QObject(parent)
{
    qDebug() << "子线程id:" << QThread::currentThreadId();

    recordGSettings = new QGSettings(KYLINRECORDER);
}


/**
 * @brief AsrThread::pathSetting
 * 默认录音位置: ~/音乐/xxx.pcm
 * 播放pcm文件可通过ffplay: ffplay -ar 48000  -ac 1 -f s16le -i xxx.pcm
 * 中间缓存文件： ~/.cache/xxx.raw
 *
 * @return
 */
QString AsrThread::pathSetting()
{
    QString outputName; //输出的文件路径名
    QString pathStr = recordGSettings->get("path").toString();
    qDebug() << "存储的路径是:" << pathStr;

    default_Location = setDefaultPath(pathStr); //设置默认存储到录音的路径
    fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    recordTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    if (recordGSettings->get("path").toString() == "" || pathStr.split("/").last() == "") {
        qDebug() << "配置文件中路径:" << recordGSettings->get("path").toString()
                 << "最后一个'/'后面的字符串是" << pathStr.split("/").last();

        if (recordGSettings->get("type").toInt() == 1) {
            outputName = default_Location + tr("/") + fileName + tr(".pcm");
            format = Mp3();
        } else if (recordGSettings->get("type").toInt() == 2) {
            outputName = default_Location + tr("/") + fileName + tr(".wav");
            format = Wav();
        } else {
            outputName = default_Location + tr("/") + fileName + tr(".m4a");
            format = M4a();
        }
    } else {
        QString tempStr = recordGSettings->get("path").toString();
        qDebug() << "尾部'/'后有字符串的路径:" << tempStr;

        if (recordGSettings->get("type").toInt() == 1) {
            outputName = tempStr + tr("/") + fileName + tr(".pcm");
            format = Mp3();
        } else if (recordGSettings->get("type").toInt() == 2) {
            outputName = tempStr + tr("/") + fileName + tr(".wav");
            format = Wav();
        } else {
            outputName = tempStr + tr("/") + fileName + tr(".m4a");
            format = M4a();
        }
    }
    return outputName;
}

//获得buffer等级
QVector<qreal> AsrThread::getBufferLevels(const QAudioBuffer &buffer)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;

    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i) {
            values[i] = qAbs(values.at(i) - peak_value / 2) / (peak_value / 2);
        }
        break;

    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            values = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;

    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }

    return values;
}

/**
 * 此函数用于返回给定音频格式的最大可能采样值
 */
qreal AsrThread::getPeakValue(const QAudioFormat &format)
{
    //注意：只支持最常见的样本格式
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // 其他样本格式不支持
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    return qreal(0);
}

template <class T> QVector<qreal> AsrThread::getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}

/**
 * 按压录音按钮之后触发
 * 默认录音位置: ~/音乐/xxx.pcm
 * 播放pcm文件可通过ffplay: ffplay -ar 48000  -ac 1 -f s16le -i xxx.pcm
 * 中间缓存文件： ~/.cache/xxx.raw
 */
void AsrThread::startRecord()
{
    qDebug() << "开始录音:";
    /*
     * 只要按压按钮了就说明要录音开始isRecordStart = true
     * 决定此值的只有停止按钮才可以使其置为isRecordStart = false
     * 暂停不影响此值变化
     */
    isRecordStart = true;
    //开始前，count先从0开始
    outputFileName = pathSetting(); //根据配置文件更新录音文件的路径
    qDebug() << "存储的路径是:" << outputFileName;

    endFileName = outputFileName;
    qDebug() << "录音开始";

    rawFileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".raw";
    file = new QFile();
    QString str = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    absolutionPath = str + "/.cache/" + rawFileName; // raw缓存文件放在绝对路径
    qDebug() << "绝对路径:" << absolutionPath;
    file->setFileName(absolutionPath);
    bool is_open = file->open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!is_open) {
        qDebug() << "打开失败";
        exit(1);
    }


    QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
    formatSound = inputDevice.nearestFormat(formatSound); //读时用默认的8000采样率防止波形刷新过慢
    audioDevice = monitorVoiceSource(recordGSettings->get("source").toInt());
    if (audioDevice.isNull()) {
        audioDevice = inputDevice;
    }

    audioInputSound = new QAudioInput(audioDevice, formatSound, this); //要在InitMonitor()中的audioInputSound->start()前面
    initMonitor();

    //写文件时用匹配的format采样率48000
    audioInputFile = new QAudioInput(audioDevice, Wav(), this);

    audioInputFile->setVolume(1.0); //设置录制进文件的音量大小, 1.0 代表满音量录制
    audioInputFile->start(file);
}

// Mp3格式
QAudioFormat AsrThread::Mp3()
{
    QAudioFormat format = QAudioFormat();
    format.setSampleRate(48000); /*
                                  *每秒钟对声音的采样次数，越大越精细，48000HZ(标准CD级)，
                                  *48000HZ的采样率(每秒钟采集48000个声波的点)
                                  */
    format.setChannelCount(2);   //立体声，数目为2
    format.setSampleSize(16);
    format.setCodec("audio/pcm");                    //编码器
    format.setByteOrder(QAudioFormat::LittleEndian); //低位优先
    format.setSampleType(QAudioFormat::SignedInt);   // QAudioFormat::SignedInt

    return format;
}
// Wav格式
QAudioFormat AsrThread::Wav()
{
    QAudioFormat format = QAudioFormat();            //设置采样格式
    format.setSampleRate(48000);                     /*
                                                      *采样率，每秒钟对声音的采样次数，越大越精细，
                                                      *48000HZ的采样率(每秒钟采集48000个声波的点)
                                                      */
    format.setChannelCount(1);                       //设置通道数：mono平声道，数目为1;stero立体声声道数目为2
    format.setSampleSize(16);                        //设置采样大小，一般为8位或16位
    format.setCodec("audio/pcm");                    //编码器
    format.setByteOrder(QAudioFormat::LittleEndian); //设置字节序，低位优先
    format.setSampleType(QAudioFormat::SignedInt);   //设置样本数据类型
    return format;
}

//格式M4a格式
QAudioFormat AsrThread::M4a()
{
    QAudioFormat format = QAudioFormat();
    format.setSampleRate(48000); /*
                                  *每秒钟对声音的采样次数，越大越精细，
                                  *48000HZ的采样率(每秒钟采集48000个声波的点)
                                  */
    format.setChannelCount(2);   //立体声，数目为2
    format.setSampleSize(16);
    format.setCodec("audio/pcm");                    //编码器
    format.setByteOrder(QAudioFormat::LittleEndian); //低位优先
    format.setSampleType(QAudioFormat::SignedInt);   // v10.1规定QAudioFormat::SignedInt。
    return format;
}



/**
 * @brief AsrThread::stopRecord
 * 停止录音
 */
void AsrThread::stopRecord()
{
    if (audioInputFile != nullptr) {
        audioInputFile->stop(); // 音频文件写入停止
    }

    if (audioInputSound != nullptr) {
        audioInputSound->stop(); // 监听停止
    }

    if (file != nullptr) {
        file->close(); // 停止写入录音文件
    }

    if (isRecordStart == true) {
        isRecordStart = false;
        qDebug() << "absolutionPath = "<< absolutionPath
                 << "endFileName = " << endFileName.toLocal8Bit().data();

        int num = 0;
        if (recordGSettings->get("type").toInt() == 1) { // pcm
            // 为了语音平台，测试保存到文件，采用的就是这种
            num = toConvertPCM(absolutionPath, endFileName.toLocal8Bit().data());

        } else if (recordGSettings->get("type").toInt()  == 2 ) {
            num = toConvertWAV(absolutionPath, endFileName.toLocal8Bit().data());

        } else {
            num = toConvertWAV(absolutionPath, endFileName.toLocal8Bit().data());
        }

        if (num >= 0) {
            qDebug() << "保存成功";
        }

        QTextCodec *code = QTextCodec::codecForName("gb2312"); //解决中文路径保存
        code->fromUnicode(endFileName).data();
    }


    qDebug() << "成功停止";

    audioInputFile->deleteLater();
    audioInputFile = nullptr;
    audioInputSound->deleteLater();
    audioInputSound = nullptr;
    file->deleteLater();
    file = nullptr;
}

/**
 * 设置默认存储路径判断中英文环境下文件路径唯一
 */
QString AsrThread::setDefaultPath(QString path)
{
    qDebug() << "path = " << path;

    QDir *record = new QDir;
    QLocale locale = QLocale::system().name();
    QString savePath;

    int flag = 0;

    if (path == "") {
        path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0);
        flag = 1;
    }
    bool existEn = record->exists(path + "/Recorder");
    bool existCH = record->exists(path + "/录音");
    //判断是否存在此路径,若两种路径都不存在就创建一个
    if (!existEn && !existCH) {
        //创建record文件夹
        if (locale.language() == QLocale::English) {
            //            qDebug()<<"English Environment:"<<path+"/"+tr("Recorder");
            record->mkdir(path + "/" + tr("Recorder"));
        } else if (locale.language() == QLocale::Chinese) {
            //            qDebug()<<"中文环境:"<<path+"/"+tr("Recorder");
            record->mkdir(path + "/" + tr("Recorder"));
        }
        savePath = path + "/" + tr("Recorder");
    } else {
        if (existEn) {
            savePath = path + "/Recorder";
        } else if (existCH) {
            savePath = path + "/录音";
        }
    }

    if (flag == 1) {
        recordGSettings->set("path", savePath);
        recordGSettings->set("recorderpath", savePath);
    }

    return savePath;
}


/**
 * @brief AsrThread::getRecordPathByGSettings
 * 得到录音路径
 * @return
 */
QString AsrThread::getRecordPathByGSettings()
{
    QString str = "";
    if (recordGSettings != nullptr) {
        QStringList keyList = recordGSettings->keys();
        if (keyList.contains("recorderpath")) {
            str = recordGSettings->get("recorderpath").toString();
        }
    }
    return str;
}

void AsrThread::setRecordPathByGsettings(QString filePath)
{

    recordGSettings->set("recorderpath", filePath);
}


qint64 AsrThread::toConvertPCM(QString catheFileName, QString pcmFileName)
{
    QFile cacheFile(catheFileName);
    QFile pcmFile(pcmFileName);
    if (!cacheFile.open(QIODevice::ReadWrite)) {
        return -1;
    }
    if (!pcmFile.open(QIODevice::WriteOnly)) {
        return -2;
    }

    qint64 nFileLen = cacheFile.bytesAvailable();
    pcmFile.write(cacheFile.readAll());


    cacheFile.close();
    pcmFile.close();
    if (nFileLen >= 0) {
        QFile::remove(catheFileName);
    }
    return nFileLen;

}

/**
 * @brief AsrThread::toConvertWAV
 * 通过文件 将raw 格式转换成无损的wav格式音频
 * 相比较pcm格式，wav就是多了一个头
 * @param catheFileName
 * @param filename
 * @return
 */
qint64 AsrThread::toConvertWAV(QString catheFileName, QString filename)
{
    WAVFILEHEADER WavFileHeader;
    qstrcpy(WavFileHeader.RiffName, "RIFF"); // RIFF 头(标志)
    qstrcpy(WavFileHeader.WavName, "WAVE"); // WAVE 文件
    qstrcpy(WavFileHeader.FmtName, "fmt "); // fmt 波形格式
    qstrcpy(WavFileHeader.DATANAME, "data"); // catheFileName , QString wavF数据块中的块头

    WavFileHeader.FmtLength = 16;     // 表示 FMT 的长度
    WavFileHeader.AudioFormat = 1;    // 这个表示 PCM 编码无压缩;
    WavFileHeader.BitsPerSample = 16; // 每次采样得到的样本数据位数;
    WavFileHeader.ChannleNumber = 1;  // 音频通道数平声道
    WavFileHeader.SampleRate = 48000; // 采样频率

    WavFileHeader.BytesPerSample = WavFileHeader.ChannleNumber * WavFileHeader.BitsPerSample / 8; // 数据块对齐单位(每个采样需要的字节数 = 通道数 × 每次采样得到的样本数据位数 / 8 )
    WavFileHeader.BytesPerSecond = WavFileHeader.SampleRate * WavFileHeader.BitsPerSample;   // 波形数据传输速率或(每秒平均字节数) = 采样频率 × 通道数 × 每次采样得到的样本数据位数 / 8  = 采样频率 × 每个采样需要的字节数


    QFile cacheFile(catheFileName);
    QFile wavFile(filename);
    if (!cacheFile.open(QIODevice::ReadWrite)) {
        return -1;
    }
    if (!wavFile.open(QIODevice::WriteOnly)) {
        return -2;
    }
    int nSize = sizeof(WavFileHeader);
    qint64 nFileLen = cacheFile.bytesAvailable();

    WavFileHeader.RiffLength = static_cast<unsigned long>(nFileLen - 8 + nSize); // 从下一个开始到文件结尾的字节数
    WavFileHeader.DataLength = static_cast<unsigned long>(nFileLen); // 实际音频数据的大小

    // 先将wav文件头信息写入，再将音频数据写入
    wavFile.write((const char *)&WavFileHeader, nSize);
    wavFile.write(cacheFile.readAll());


    cacheFile.close();
    wavFile.close();
    if (nFileLen >= 0) {
        QFile::remove(catheFileName);
    }
    return nFileLen;
}

/**
 * @brief AsrThread::toConvertMp3
 * pcm转换mp3
 *
 * @param catheFileName
 * @param mp3FileName
 * @return
 */
qint64 AsrThread::toConvertMp3(QString catheFileName, QString mp3FileName)
{
    QFile cacheFile(catheFileName);
    QFile mp3File(mp3FileName);

    if (!cacheFile.open(QIODevice::ReadWrite)) {
        return -1;
    }
    if (!mp3File.open(QIODevice::WriteOnly)) {
        return -2;
    }

    qint64 nFileLen = cacheFile.bytesAvailable();
    QByteArray ba = cacheFile.readAll();

    qDebug() << "缓存长度:" << ba.size() << nFileLen;
    QTextCodec *code = QTextCodec::codecForName("gb2312"); //解决中文路径保存
    code->fromUnicode(mp3FileName).data();
    cacheFile.close();
    mp3File.close();

    return nFileLen;
}

/**
 * @brief AsrThread::pauseRecord
 * 暂停录制声音
 */
void AsrThread::pauseRecord()
{
    audioInputFile->stop();
    audioInputSound->stop();
}

void AsrThread::playRecord()
{

    audioInputFile->start(file);
    inputIODevice = audioInputSound->start();
    connect(inputIODevice, SIGNAL(readyRead()), this, SLOT(readyReadRecordData())); //点击开始按钮时录制声音
}

/**
 * @brief AsrThread::InitMonitor
 * 判断当前的音频输入设备是否支持QAudioFormat配置，如果不支持，获取支持的最接近的配置信息
 */
void AsrThread::initMonitor()
{
    inputIODevice = audioInputSound->start();

    connect(inputIODevice, SIGNAL(readyRead()), this, SLOT(readyReadRecordData()));
}

/**
 * @brief AsrThread::monitorVoiceSource
 * 当前录音设备的来源
 * @param sourceType
 *      1: 全部
 *      2: 系统内部
 *      3: 麦克风
 *
 * @return
 */
QAudioDeviceInfo AsrThread::monitorVoiceSource(int sourceType)
{
    qDebug() << "配置文件中的值(1.全部2.内部3.麦克风)" << sourceType;

    if (sourceType == 2)  {
        //当录制系统内部声音时用此方法
        qDebug() << "2: 系统内部" << sourceType;

        QList<QAudioDeviceInfo> audioDeviceListI = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
        foreach (QAudioDeviceInfo devInfo, audioDeviceListI) {
            qDebug()<<"输入设备:"<<devInfo.deviceName();

            if (devInfo.deviceName().contains("monitor")) {
                qDebug() << "当前内部输入设备:" << devInfo.deviceName();
                audioDevice = devInfo;
            }
        }

        return audioDevice;
    } else  {
        // 其他，如麦克风
        qDebug() << "3: 麦克风声音";
        QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
        formatSound = inputDevice.nearestFormat(formatSound);
        qDebug() << "当前外部输入设备:" << inputDevice.deviceName() << formatSound; //可以判断当前输入设备

        return inputDevice;
    }
}

/**
 * @brief AsrThread::readyReadRecordData
 * 读取录音数据
 */
void AsrThread::readyReadRecordData()
{
    //如果输入为空则返回空
    if (!audioInputSound) {
        qDebug() << "输入为空!";
        return;
    }

    //处理音频输入，recordBuffer中保存音频数据，len是获取到的数据的长度
    QByteArray recordBuffer(BufferSize, 0);
    len = audioInputSound->bytesReady();
    if (len > 4096) {
        len = 4096;
    }

    // 真正进行将录音数据保存到recordBuffer，每次传输的pcm数据就是recordBuffer
    qint64 l = inputIODevice->read(recordBuffer.data(), len);
    if (l <= 0) {
        return;
    }

    // 样本数？

    // 以下是为了保存到文件中才进行的操作
    short *tempData = (short *)recordBuffer.data();
    outdata = tempData;
    MaxValue = 0;
    for (int i = 0; i < len; i++) {
        // 把样本数据转换为整型
        value = abs(useVolumeSample( outdata[i])); // 麦克风中的薄膜始终是在平衡位置附近value会检测到正负相间的震荡频率,加个绝对值
        MaxValue = MaxValue >= value ? MaxValue : value;
    }

    // 每检测到value就可发送一次信号

    file->flush();

    /*
     * fsync(file->handle());
     * 将所有修改过的块缓冲区写入队列，然后就返回，并不等待实际写操作的结束
     * 同步内存中的所有已修改的文件数据到存储设备(必须加，否则断电时文件就无法正确保存)
     */
    fsync(file->handle());
}

/**
 * @brief AsrThread::useVolumeSample
 * 求每次检测到的振幅组中最大值
 *
 * @param sample
 * @return
 */
int AsrThread::useVolumeSample(qreal sample)
{
    return qMax(qMin(((sample * soundVolume) / 100), 30000.0), -30000.0);
}
