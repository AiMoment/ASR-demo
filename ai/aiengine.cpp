#include "aiengine.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>

#ifdef WEBSOCKET_AI
#include "websocketclientmanager.h"
#endif

AIEngine::AIEngine(QObject *parent) : QObject(parent)
{
    m_isEnable = false;
#ifdef WEBSOCKET_AI
    initWebSocket();
#endif
}

int AIEngine::feedData(char *_data, int _size)
{
    if (!m_isEnable)
        return -1;
#ifdef WEBSOCKET_AI
    wscm->sendBinaryMessage(_data, _size);
#endif
}

void AIEngine::enable()
{
    qDebug() << "开始识别 before startDistinguish";
    wscm->startDistinguish();
    m_isEnable = true;
}

void AIEngine::disable()
{
    wscm->endDistinguish();
    m_isEnable = false;
}

#ifdef WEBSOCKET_AI
void AIEngine::initWebSocket()
{
    wscm = new WebSocketClientManager;
    connect(wscm, &WebSocketClientManager::textMessageReceivedSignal, this, &AIEngine::textMessageReceivedParseSlot);

    string url = wscm->getAuthorizeUrl();

    QString realurl = QString::fromStdString(url);

    wscm->setUrl(realurl);
}

/**
 * @brief AIEngine::textMessageReceivedParseSlot 解析从科大讯飞服务器获得的语音识别返回JSON数据
 * @param str  从科大讯飞服务器获得的语音识别返回JSON数据
 */
void AIEngine::textMessageReceivedParseSlot(QString str)
{
    QJsonDocument json = QJsonDocument::fromJson(str.toLocal8Bit().data());

    if (json["code"] == 0) {
        QString result = json["data"].toString();

        qDebug() << "开始解析JSON数据： " << result;

        emit translateOk(result);
    }
}
#endif

