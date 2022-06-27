#include "websocketclientmanager.h"

#include <QFile>
#include <QThread>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <time.h>


// user: yushuoqi
// https://console.bce.baidu.com/ai/#/ai/speech/app/detail~appId=3267504
const char *APPID = "898a0b0c"; // 修改为你自己网页上鉴权参数的appid
const char* APPKEY = "3127f74200187b000bde672967b4d4a4"; // 修改为你自己网页上鉴权参数的appkey

// 修改其它识别语言或者识别模型
const int DEV_PID = 15372;
const string HOST = "vop.baidu.com";
const int PORT = 80;
const string PATH = "/realtime_asr";


WebSocketClientManager::WebSocketClientManager(QObject *parent)
    : QObject(parent),
      enable(false),
      start(false),
      _running(false),
      _pWebSocket(nullptr),
      _connected(false)
{
    initSocket();
}

WebSocketClientManager::~WebSocketClientManager()
{
    if(_pWebSocket != 0)
    {
        _pWebSocket->deleteLater();
        _pWebSocket = 0;
    }
}

bool WebSocketClientManager::running() const
{
    return _running;
}

void WebSocketClientManager::slot_stop()
{
    if(!_running)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not running...";
        return;
    }
    _running = false;
    _pWebSocket->close();
}

/**
 * @brief WebSocketClientManager::slot_sendTextMessage 开始发送text数据
 * @param message
 */
void WebSocketClientManager::slot_sendTextMessage(const QString &message)
{
    qDebug() << "开始发送数据 slot_sendTextMessage message: " << message;
    if(!_running)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not running...";
        return;
    }
    bool result = true;

    int sendStatus = _pWebSocket->sendTextMessage(message);
    qDebug() << "发送文本数据状态： " << sendStatus;

    emit signal_sendTextMessageResult(result);
}

void WebSocketClientManager::slot_sendBinaryMessage(const QByteArray &data)
{
    if(!_running)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not running...";
        return;
    }
    bool result = true;
    _pWebSocket->sendBinaryMessage(data);
    _pWebSocket->flush();
//    qDebug() << "send binary message";
    emit signal_sendBinaryMessageResult(result);
}

static QFile f;
static QDataStream stream;

void WebSocketClientManager::slot_connected()
{
    _connected = true;
    qDebug() << __FILE__ << __LINE__ << "slot_connected: 响应连接成功";

    // 是否从 enable 连接的
    if (start) {
        enable = true;
        qDebug() << "发送开始头";
        // 发送开始头
        QJsonObject json;

        QJsonObject commonJson;
        commonJson.insert("app_id", QString::fromStdString(COMMON.APPID));

        QJsonObject businessJson;
        businessJson.insert("language", QString::fromStdString(BUSINESS.language));
        businessJson.insert("domain", QString::fromStdString(BUSINESS.domain));
        businessJson.insert("accent", QString::fromStdString(BUSINESS.accent));

        QJsonObject dataJson;
        dataJson.insert("status", STATUS_FIRST_FRAME);
        dataJson.insert("format", QString::fromStdString(DATA.format));
        dataJson.insert("encoding", QString::fromStdString(DATA.encoding));
        dataJson.insert("audio", QString::fromStdString(get_base64_encode("")));

        json.insert("common", commonJson);
        json.insert("business", businessJson);
        json.insert("data", dataJson);
//        qDebug() << QString::fromStdString(QJsonDocument(json).toJson().toStdString());
        slot_sendTextMessage(QString::fromStdString(QJsonDocument(json).toJson().toStdString()));

        qDebug() << "before create thread.";

        QThread::create([this]{
            while (enable) {
                QThread::msleep(10);
                if (_dataQueue.size() > 0) {
                    QByteArray bya = _dataQueue.front();
                    _dataQueue.pop_front();
                    sig_send(bya);
                }
            }
        })->start();
    }
    emit signal_connected();
}

void WebSocketClientManager::slot_disconnected()
{
    _connected = false;
    qDebug() << __FILE__ << __LINE__ << "disconnected";
    emit signal_disconnected();
}

void WebSocketClientManager::slot_error(QAbstractSocket::SocketError error)
{
    qDebug() << __FILE__ << __LINE__ << (int)error << _pWebSocket->errorString();
    emit signal_error(_pWebSocket->errorString());
}

void WebSocketClientManager::onSslErrors(const QList<QSslError> &errors)
{

    qDebug() << __FILE__ << __LINE__ << errors;
    Q_UNUSED(errors);

    _pWebSocket->ignoreSslErrors();

}

void WebSocketClientManager::slot_textFrameReceived(const QString &frame, bool isLastFrame)
{
    qDebug() << "slot_textFrameReceived: "
             << "frame = " << frame
             << "isLastFrame =  " << isLastFrame;

    emit signal_textFrameReceived(frame, isLastFrame);
}

void WebSocketClientManager::slot_textMessageReceived(const QString &message)
{
    qDebug() << "message = " << message;
    emit signal_textMessageReceived(message);
}

void WebSocketClientManager::initSocket()
{
    qDebug() << "initSocket";
    if (_running)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__ << "it's already running...";
        return;
    }
    if (!_pWebSocket)
    {
        qDebug() << "开始创建socket";
        _pWebSocket = new QWebSocket();

        QSslConfiguration config = _pWebSocket->sslConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1SslV3);
        _pWebSocket->setSslConfiguration(config);

        connect(_pWebSocket, SIGNAL(connected())   , this, SLOT(slot_connected()));
        connect(_pWebSocket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
        connect(_pWebSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this       , SLOT(slot_error(QAbstractSocket::SocketError)));
        connect(_pWebSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
                this       , &WebSocketClientManager::onSslErrors);
        connect(_pWebSocket, SIGNAL(textFrameReceived(QString,bool)),
                this       , SLOT(slot_textFrameReceived(QString,bool)));
        connect(_pWebSocket, SIGNAL(textMessageReceived(QString)),
                this       , SLOT(slot_textMessageReceived(QString)));
        connect(this, &WebSocketClientManager::sig_send, this, &WebSocketClientManager::slot_sendBinaryMessage);
    }
    _running = true;
}

QString WebSocketClientManager::url() const
{
    return _url;
}

/**
 * @brief WebSocketClientManager::get_url 获得建立连接的鉴权url
 * @return 鉴权url
 */
string WebSocketClientManager::get_url()
{
    API.APIKey = "3127f74200187b000bde672967b4d4a4";
    API.APISecret = "NGU2NGY4YmVhYTg5NzE2ZjlkYzhiMDhk";

    COMMON.APPID = "898a0b0c";

    BUSINESS.language = "zh_cn";
    BUSINESS.domain = "iat";
    BUSINESS.accent = "mandarin";

    DATA.format = "audio/L16;rate=16000";
    DATA.encoding = "raw"; // 测试raw, opus, opus-wb, speex, speex-wb通过

    OTHER.audio_file = "/home/yushuoqi/code/ASR-demo/iat_pcm_16k.pcm";


    // 生成RFC1123格式的时间戳，"Thu, 05 Dec 2019 09:54:17 GMT"
    setlocale(LC_TIME,"en_US.UTF-8"); // 设置为英文日期编码格式
    time_t rawtime = time(NULL);
    char buf[1024];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %X %Z", gmtime(&rawtime));
    string date = string(buf);
//    string date = string("Fri, 24 Jun 2022 02:28:00 GMT"); // 403 发现是中文格式问题

//    qDebug() << "date: " << QString::fromStdString(date);

    // 拼接signature的原始字符串
//    string signature_origin = "host: ws-api.xfyun.cn\n";
    string signature_origin = "host: iat-api.xfyun.cn\n";
    signature_origin += "date: " + date + "\n";
    signature_origin += "GET /v2/iat HTTP/1.1";

//    string signature_origin_utf8 = QString::fromStdString(signature_origin).toUtf8().data();

    // 使用hmac-sha256算法结合apiSecret对signature_origin签名，获得签名后的摘要signature_sha
    string signature_sha = get_hmac_sha256(signature_origin, API.APISecret);

    // 使用base64编码对signature_sha进行编码获得最终的signature
    string signature = get_base64_encode(signature_sha);

    // 拼接authorization的原始字符串
    sprintf(buf, "api_key=\"%s\", algorithm=\"%s\", headers=\"%s\", signature=\"%s\"",
            API.APIKey.c_str(), "hmac-sha256", "host date request-line", signature.c_str());
    string authorization_origin = string(buf);
//    qDebug() << "authorization_origin= " << QString::fromStdString(authorization_origin);

    // 再对authorization_origin进行base64编码获得最终的authorization参数
    string authorization = get_base64_encode(authorization_origin);

    // 对相关参数构成url，并进行url编码，生成最终鉴权url
    sprintf(buf, "authorization=%s&date=%s&host=%s",
            authorization.c_str(), date.c_str(), "iat-api.xfyun.cn");

    string url = "wss://iat-api.xfyun.cn/v2/iat?" + get_url_encode(string(buf));

    qDebug() << "url= " << QString::fromStdString(url);

    return url;

}

void WebSocketClientManager::setUrl(const QString &url)
{
    _url = url;
}

void WebSocketClientManager::connectUrl(QString url)
{
    if (!_running)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not running...";
        return;
    }
    _url = url;
    _pWebSocket->open(QUrl(url));
}

void WebSocketClientManager::startDistinguish()
{
    qDebug() << "startDistinguish 开始识别";
    start = true;
    if (_pWebSocket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "已经正确连接情况下，进行发送开始头";
        enable = true;
        // 发送开始头
        QJsonObject json;

        QJsonObject commonJson;
        commonJson.insert("app_id", QString::fromStdString(COMMON.APPID));

        QJsonObject businessJson;
        businessJson.insert("language", QString::fromStdString(BUSINESS.language));
        businessJson.insert("domain", QString::fromStdString(BUSINESS.domain));
        businessJson.insert("accent", QString::fromStdString(BUSINESS.accent));

        QJsonObject dataJson;
        dataJson.insert("status", STATUS_FIRST_FRAME);
        dataJson.insert("format", QString::fromStdString(DATA.format));
        dataJson.insert("encoding", QString::fromStdString(DATA.encoding));
        dataJson.insert("audio", QString::fromStdString(get_base64_encode("")));

        json.insert("common", commonJson);
        json.insert("business", businessJson);
        json.insert("data", dataJson);
        qDebug() << QString::fromStdString(QJsonDocument(json).toJson().toStdString());
        slot_sendTextMessage(QString::fromStdString(QJsonDocument(json).toJson().toStdString()));

        QThread::create([this]{
            while (enable) {
                QThread::msleep(10);
                if (_dataQueue.size() > 0) {
                    QByteArray bya = _dataQueue.front();
                    _dataQueue.pop_front();
                    sig_send(bya);
                }
            }
        })->start();
    }
    else {
        qDebug() << "开始识别时，发现没有正确连接，因此打开以下地址： " << _url;
        _pWebSocket->open(QUrl(_url));
    }
}

// 停止识别
void WebSocketClientManager::endDistinguish()
{
    start = false;
    enable = false;
    if (!_connected) {
        qDebug() << "end error, socket not connected";
        return;
    }

    QJsonObject json;
    QJsonObject stopJson;
    stopJson.insert("status", STATUS_LAST_FRAME);
    // extend
    stopJson.insert("format", QString::fromStdString(DATA.format));
    stopJson.insert("encoding", QString::fromStdString(DATA.encoding));
    stopJson.insert("audio", QString::fromStdString(get_base64_encode("")));

    json.insert("data", stopJson);
    qDebug() << QString::fromStdString(QJsonDocument(json).toJson().toStdString());
    slot_sendTextMessage(QString::fromStdString(QJsonDocument(json).toJson().toStdString()));

    _pWebSocket->close();
    _dataQueue.clear();
}

void WebSocketClientManager::sendBinaryMessage(char* data, int len)
{
    QByteArray bya(data, len);
    _dataQueue.push_back(bya);
}
