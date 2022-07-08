#include "websocketclientmanager.h"

#include <QFile>
#include <QThread>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <time.h>
#include <pthread.h>

const int g_frameSize = 1280; //每一帧音频大小的整数倍，每次发送音频字节数1280B
const int g_intervel = 40; // 未压缩的PCM格式，每次发送音频间隔40ms

pthread_mutex_t popMutex;

WebSocketClientManager::WebSocketClientManager(QObject *parent)
    : QObject(parent),
      m_isEnabled(false),
      m_isStarted(false),
      m_isRunning(false),
      m_pWebSocket(nullptr),
      m_isConnected(false)
{
    initSocket();
}

WebSocketClientManager::~WebSocketClientManager()
{
    if(m_pWebSocket != 0)
    {
        m_pWebSocket->deleteLater();
        m_pWebSocket = 0;
    }
}

bool WebSocketClientManager::isRunning() const
{
    return m_isRunning;
}

/**
 * @brief WebSocketClientManager::stopSlot 停止websocket
 */
void WebSocketClientManager::stopSlot()
{
    if(!m_isRunning)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not isRunning...";
        return;
    }
    m_isRunning = false;
    m_pWebSocket->close();
}


/**
 * @brief WebSocketClientManager::sendTextMessageSlot 开始发送text数据
 * 科大讯飞语音识别接口，第一帧、中间帧和最后一帧都是发送TextMessage
 *
 * @param message
 */
void WebSocketClientManager::sendTextMessageSlot(const QString &message)
{
    qDebug() << "开始发送数据 message: " << message;
    if(!m_isRunning) {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not isRunning...";
        return;
    }
    bool result = true;

    int sendStatus = m_pWebSocket->sendTextMessage(message);
    qDebug() << "发送文本数据状态： " << sendStatus;

    emit sendTextMessageResultSignal(result);
}

/**
 * @brief WebSocketClientManager::slot_sendBinaryMessage 百度语音识别 帧的类型（Opcode）是Binary
 * 内容是二进制的音频内容。 除最后一个音频数据帧， 每个帧的音频数据长度为20-200ms。
 *
 * @param data 音频数据
 */
void WebSocketClientManager::sendBinaryMessageSlot(const QByteArray &data)
{
    if(!m_isRunning)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not isRunning...";
        return;
    }
    bool result = true;
    m_pWebSocket->sendBinaryMessage(data);
    m_pWebSocket->flush();

    emit sendBinaryMessageResultSignal(result);
}


/**
 * @brief WebSocketClientManager::connectedSlot 响应连接
 */
void WebSocketClientManager::connectedSlot()
{
    m_isConnected = true;
    qDebug() << __FILE__ << __LINE__ << "connectedSlot: 响应连接成功";

    // 是否从 enable 连接的
    if (m_isStarted) {
        m_isEnabled = true;
        qDebug() << "发送开始头";

        QString firstFrameText = makeFirstFrameText("");
        sendTextMessageSlot(firstFrameText);

        QThread::create([this]{
            while (m_isEnabled) {
                QThread::msleep(g_intervel);

                pthread_mutex_lock(&popMutex);

                if (m_audioDataQueue.size() > 0) {
                    QByteArray bya = m_audioDataQueue.front();
                    m_audioDataQueue.pop_front();
                    QString sendText = makeContinueFrameText(bya);
                    sendContinueFrameAudioDataSignal(sendText);
                }
                pthread_mutex_unlock(&popMutex);
            }
        })->start();
    }
    emit connectedSignal();
}

/**
 * @brief WebSocketClientManager::disconnectedSlot 断开连接
 */
void WebSocketClientManager::disconnectedSlot()
{
    m_isConnected = false;

    qDebug() << __FILE__ << __LINE__ << "disconnected";

    emit disconnectedSignal();
}

/**
 * @brief WebSocketClientManager::errorSlot 连接错误
 * @param error
 */
void WebSocketClientManager::errorSlot(QAbstractSocket::SocketError error)
{
    qDebug() << __FILE__ << __LINE__ << (int)error << m_pWebSocket->errorString();
    emit errorSignal(m_pWebSocket->errorString());
}

/**
 * @brief WebSocketClientManager::sslErrorSlot ssl连接错误
 * @param errors
 */
void WebSocketClientManager::sslErrorSlot(const QList<QSslError> &errors)
{

    qDebug() << __FILE__ << __LINE__ << errors;

    Q_UNUSED(errors);

    m_pWebSocket->ignoreSslErrors();

}

/**
 * @brief WebSocketClientManager::slot_textMessageReceived 远程服务器返回信息
 * @param message 远程服务器返回信息，一般为JSON格式
 */
void WebSocketClientManager::textMessageReceivedSlot(const QString &message)
{
    qDebug() << "讯飞语音识别服务器返回 message = " << message;
    emit textMessageReceivedSignal(message);
}

void WebSocketClientManager::textFrameReceivedSlot(const QString &frame, bool isLastFrame)
{
    emit textFrameReceivedSignal(frame, isLastFrame);
}

/**
 * @brief WebSocketClientManager::initSocket 初始化websocket
 * 由于科大讯飞需要wss，需支持ssl，因此这里采用ssl方式连接
 */
void WebSocketClientManager::initSocket()
{
    qDebug() << "initSocket";
    if (m_isRunning) {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__ << "it's already isRunning...";
        return;
    }

    if (!m_pWebSocket) {
        qDebug() << "开始创建 socket";
        m_pWebSocket = new QWebSocket();

        QSslConfiguration config = m_pWebSocket->sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1SslV3);
        m_pWebSocket->setSslConfiguration(config);

        connect(m_pWebSocket, SIGNAL(connected())   , this, SLOT(connectedSlot()));
        connect(m_pWebSocket, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));

        connect(m_pWebSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSlot(QAbstractSocket::SocketError)));
        connect(m_pWebSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this       , &WebSocketClientManager::sslErrorSlot);

        connect(m_pWebSocket, SIGNAL(textFrameReceived(QString,bool)), this, SLOT(textFrameReceivedSlot(QString,bool)));
        connect(m_pWebSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageReceivedSlot(QString)));

        connect(this, &WebSocketClientManager::sendContinueFrameAudioDataSignal, this, &WebSocketClientManager::sendTextMessageSlot);

        connect(this, &WebSocketClientManager::sendDirectAudioDataSignal, this, &WebSocketClientManager::sendBinaryMessageSlot);
    }

    m_isRunning = true;
}

QString WebSocketClientManager::getUrl() const
{
    return m_authorizeUrl;
}

/**
 * @brief WebSocketClientManager::getAuthorizeUrl 获得建立连接的鉴权Url
 * @return 鉴权Url
 */
string WebSocketClientManager::getAuthorizeUrl()
{
    m_apiInfo.APIKey = "3127f74200187b000bde672967b4d4a4";
    m_apiInfo.APISecret = "NGU2NGY4YmVhYTg5NzE2ZjlkYzhiMDhk";

    m_commonInfo.APPID = "898a0b0c";

    m_businessInfo.language = "zh_cn";
    m_businessInfo.domain = "iat";
    m_businessInfo.accent = "mandarin";

    m_dataInfo.format = "audio/L16;rate=16000";
    m_dataInfo.encoding = "raw"; // 测试raw, opus, opus-wb, speex, speex-wb通过

//    m_otherInfo.audio_file = "/home/yushuoqi/code/ASR-demo/iat_pcm_16k.pcm";
    m_otherInfo.audio_file = "";

    // 生成RFC1123格式的时间戳，"Thu, 05 Dec 2019 09:54:17 GMT",中文格式会出现401或403错误`
    setlocale(LC_TIME,"en_US.UTF-8"); // 设置为英文日期编码格式
    time_t rawtime = time(NULL);
    char buf[1024];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %X %Z", gmtime(&rawtime));
    string date = string(buf);

    // 拼接signature的原始字符串
    string signature_origin = "host: iat-api.xfyun.cn\n";
    signature_origin += "date: " + date + "\n";
    signature_origin += "GET /v2/iat HTTP/1.1";

    // 使用hmac-sha256算法结合apiSecret对signature_origin签名，获得签名后的摘要signature_sha
    string signature_sha = get_hmac_sha256(signature_origin, m_apiInfo.APISecret);

    // 使用base64编码对signature_sha进行编码获得最终的signature
    string signature = get_base64_encode(signature_sha);

    // 拼接authorization的原始字符串
    sprintf(buf, "api_key=\"%s\", algorithm=\"%s\", headers=\"%s\", signature=\"%s\"",
            m_apiInfo.APIKey.c_str(), "hmac-sha256", "host date request-line", signature.c_str());
    string authorization_origin = string(buf);

    // 再对authorization_origin进行base64编码获得最终的authorization参数
    string authorization = get_base64_encode(authorization_origin);

    // 对相关参数构成url，并进行url编码，生成最终鉴权url
    sprintf(buf, "authorization=%s&date=%s&host=%s",
            authorization.c_str(), date.c_str(), "iat-api.xfyun.cn");

    string url = "wss://iat-api.xfyun.cn/v2/iat?" + getm_authorizeUrl_encode(string(buf));

    qDebug() << "url= " << QString::fromStdString(url);

    return url;

}

void WebSocketClientManager::setUrl(const QString &url)
{
    m_authorizeUrl = url;
}

void WebSocketClientManager::connectUrl(QString url)
{
    if (!m_isRunning) {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not isRunning...";
        return;
    }
    m_authorizeUrl = url;
    m_pWebSocket->open(QUrl(url));
}

void WebSocketClientManager::startDistinguish()
{
    qDebug() << "startDistinguish 开始识别";
    m_isStarted = true;

    if (m_pWebSocket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "已经正确连接情况下，进行发送开始头";
        m_isEnabled = true;

        QString firstFrameText = makeFirstFrameText("");
        sendTextMessageSlot(firstFrameText);


        QThread::create([this]{
            while (m_isEnabled) {
                QThread::msleep(g_intervel);

                pthread_mutex_lock(&popMutex);

                if (m_audioDataQueue.size() > 0) {
                    QByteArray bya = m_audioDataQueue.front();
                    m_audioDataQueue.pop_front();
                    QString sendText = makeContinueFrameText(bya);
                    sendContinueFrameAudioDataSignal(sendText);
                }
                pthread_mutex_unlock(&popMutex);
            }
        })->start();
    }
    else {
        qDebug() << "开始识别时，发现没有正确连接，因此打开以下地址： " << m_authorizeUrl;
        m_pWebSocket->open(QUrl(m_authorizeUrl));
    }
}

/**
 * @brief WebSocketClientManager::endDistinguish 停止调用科大讯飞语音识别
 */
void WebSocketClientManager::endDistinguish()
{
    m_isStarted = false;
    m_isEnabled = false;

    if (!m_isConnected) {
        qDebug() << "end error, socket not connected";
        return;
    }

    QString sendLastFrameText = makeLastFrameText("");

    sendTextMessageSlot(sendLastFrameText);

    m_pWebSocket->close();
    m_audioDataQueue.clear();
}

/**
 * @brief WebSocketClientManager::makeFirstFrameText 组装讯飞语音识别的第一帧JSON数据
 * @param data 第一帧的音频数据，针对麦克风方式，可以发送为空
 * @return 组装好的第一帧音频JSON数据
 */
QString WebSocketClientManager::makeFirstFrameText(QString data)
{
    QJsonObject json;

    QJsonObject commonJson;
    commonJson.insert("app_id", QString::fromStdString(m_commonInfo.APPID));

    QJsonObject businessJson;
    businessJson.insert("language", QString::fromStdString(m_businessInfo.language));
    businessJson.insert("domain", QString::fromStdString(m_businessInfo.domain));
    businessJson.insert("accent", QString::fromStdString(m_businessInfo.accent));

    QJsonObject dataJson;
    dataJson.insert("status", STATUS_FIRST_FRAME);
    dataJson.insert("format", QString::fromStdString(m_dataInfo.format));
    dataJson.insert("encoding", QString::fromStdString(m_dataInfo.encoding));

    dataJson.insert("audio", QString::fromStdString(get_base64_encode(data.toStdString())));

    json.insert("common", commonJson);
    json.insert("business", businessJson);
    json.insert("data", dataJson);

    QString firstFrameTextJson = QString::fromStdString(QJsonDocument(json).toJson().toStdString());

    return firstFrameTextJson;
}

/**
 * @brief WebSocketClientManager::makeContinueFrameText 组装中间帧音频JSON数据
 * @param data 需要加到JSON中的音频数据
 * @return 组装好的中间帧音频数据JSON
 */
QString WebSocketClientManager::makeContinueFrameText(const QByteArray &data)
{
    QJsonObject json;

    QJsonObject commonJson;
    commonJson.insert("app_id", QString::fromStdString(m_commonInfo.APPID));

    QJsonObject businessJson;
    businessJson.insert("language", QString::fromStdString(m_businessInfo.language));
    businessJson.insert("domain", QString::fromStdString(m_businessInfo.domain));
    businessJson.insert("accent", QString::fromStdString(m_businessInfo.accent));

    QJsonObject dataJson;
    dataJson.insert("status", STATUS_CONTINUE_FRAME);
    dataJson.insert("format", QString::fromStdString(m_dataInfo.format));
    dataJson.insert("encoding", QString::fromStdString(m_dataInfo.encoding));
    dataJson.insert("audio", QString::fromStdString(get_base64_encode(data.toStdString())));

    json.insert("common", commonJson);
    json.insert("business", businessJson);
    json.insert("data", dataJson);

    QString continueFrameText =  QString::fromStdString(QJsonDocument(json).toJson().toStdString());

    return continueFrameText;

}

/**
 * @brief WebSocketClientManager::makeLastFrameText 组装結束帧音频JSON数据
 * @param data 需要加到JSON中的音频数据
 * @return 组装好的結束帧音频数据JSON
 */
QString WebSocketClientManager::makeLastFrameText(QString data)
{
    QJsonObject json;
    QJsonObject stopJson;

    stopJson.insert("status", STATUS_LAST_FRAME);
    stopJson.insert("format", QString::fromStdString(m_dataInfo.format));
    stopJson.insert("encoding", QString::fromStdString(m_dataInfo.encoding));
    stopJson.insert("audio", QString::fromStdString(get_base64_encode(data.toStdString())));

    json.insert("data", stopJson);

    QString lastFrameText = QString::fromStdString(QJsonDocument(json).toJson().toStdString());

    return lastFrameText;

}

void WebSocketClientManager::sendBinaryMessage(char* data, int len)
{
    QByteArray bya(data, len);
    m_audioDataQueue.push_back(bya);
}
