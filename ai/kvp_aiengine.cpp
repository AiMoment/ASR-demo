#include "kvp_aiengine.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>

#ifdef WEBSOCKET_AI
#include "websocketclientmanager.h"
#endif

KVP_AIEngine::KVP_AIEngine(QObject *parent) : QObject(parent)
{
    m_isEnable = false;
#ifdef WEBSOCKET_AI
    initWebSocket();
#endif
}

int KVP_AIEngine::feedData(char *_data, int _size)
{
//    emit translateOk("翻译好了");
    if (!m_isEnable)
        return -1;
#ifdef WEBSOCKET_AI
    wscm->sendBinaryMessage(_data, _size);
#endif
}

void KVP_AIEngine::enable()
{
    wscm->startDistinguish();
    m_isEnable = true;
}

void KVP_AIEngine::disable()
{
    wscm->endDistinguish();
    m_isEnable = false;
}

#ifdef WEBSOCKET_AI
void KVP_AIEngine::initWebSocket()
{
    wscm = new WebSocketClientManager;
    connect(wscm, &WebSocketClientManager::signal_textMessageReceived, this, &KVP_AIEngine::slotWebSocketMessage);
    QString url = AI_URL;
    wscm->setUrl(url);
}

void KVP_AIEngine::slotWebSocketMessage(QString str)
{
    // 解析 json
    QJsonDocument json = QJsonDocument::fromJson(str.toLocal8Bit().data());

    if (json["type"] == "FIN_TEXT") {
        QString result = json["result"].toString();
        qDebug() << result;
        emit translateOk(result);
    }
}
#endif
