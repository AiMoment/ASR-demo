#include "websocketclientmanager.h"

#include <QFile>
#include <QThread>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

//const int APPID = 25532607;
//const char* APPKEY = "QpqM5GSGUcZAvZj8P2UY1bwu";

const int APPID = 26272133; // 修改为你自己网页上鉴权参数的appid
const char* APPKEY = "ESMjcGPyzNlBMBEt7tD2bWzz"; // 修改为你自己网页上鉴权参数的appkey

// 修改其它识别语言或者识别模型
const int DEV_PID = 15372;
const std::string HOST = "vop.baidu.com";
const int PORT = 80;
const std::string PATH = "/realtime_asr";


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

void WebSocketClientManager::slot_sendTextMessage(const QString &message)
{
    if(!_running)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__
                 << ", it's not running...";
        return;
    }
    bool result = true;
    qDebug() << _pWebSocket->sendTextMessage(message);
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
    qDebug() << __FILE__ << __LINE__ << "connected";

    // 是否从 enable 连接的
    if (start) {
        enable = true;
        // 发送开始头
        QJsonObject js;
        QJsonObject json;
        js.insert("appid", APPID);
        js.insert("appkey", APPKEY);
        js.insert("dev_pid", DEV_PID);
        js.insert("cuid", "your_self_defined_user_id");
        js.insert("format", "pcm");
        js.insert("sample", 16000);

        json.insert("type", "START");
        json.insert("data", js);
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

void WebSocketClientManager::slot_textFrameReceived(const QString &frame, bool isLastFrame)
{
    emit signal_textFrameReceived(frame, isLastFrame);
}

void WebSocketClientManager::slot_textMessageReceived(const QString &message)
{
    emit signal_textMessageReceived(message);
}

void WebSocketClientManager::initSocket()
{
    if (_running)
    {
        qDebug() << __FILE__ << __LINE__
                 << "Failed to" << __FUNCTION__ << "it's already running...";
        return;
    }
    if (!_pWebSocket)
    {
        _pWebSocket = new QWebSocket();
        connect(_pWebSocket, SIGNAL(connected())   , this, SLOT(slot_connected()));
        connect(_pWebSocket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));
        connect(_pWebSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this       , SLOT(slot_error(QAbstractSocket::SocketError)));
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
    start = true;
    if (_pWebSocket->state() == QAbstractSocket::ConnectedState) {
        enable = true;
        // 发送开始头
        QJsonObject js;
        QJsonObject json;
        js.insert("appid", APPID);
        js.insert("appkey", APPKEY);
        js.insert("dev_pid", DEV_PID);
        js.insert("cuid", "your_self_defined_user_id");
        js.insert("format", "pcm");
        js.insert("sample", 16000);

        json.insert("type", "START");
        json.insert("data", js);
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

    QJsonObject js;
    QJsonObject json;
    js.insert("appid", APPID);
    js.insert("appkey", APPKEY);
    js.insert("dev_pid", DEV_PID);
    js.insert("cuid", "your_self_defined_user_id");
    js.insert("format", "pcm");
    js.insert("sample", 16000);

    json.insert("type", "FINISH");
    json.insert("data", js);
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
