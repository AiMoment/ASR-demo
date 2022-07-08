#include "aiengine.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
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
    QString resultText;

//    QJsonDocument json = QJsonDocument::fromJson(str.toLocal8Bit().data());
    QJsonDocument document;
    QJsonParseError parseJsonErr;

    document = QJsonDocument::fromJson(str.toUtf8(), &parseJsonErr);
    if (parseJsonErr.error != QJsonParseError::NoError) {
        qDebug() << "解析JSON信息出错";

        return;
    }

    qDebug() << "开始解析JSON： " << document;

    if (document.isObject()) {
        QJsonObject obj = document.object();
        QStringList keys = obj.keys();

        QList<QString>::iterator it;
        for (it=keys.begin(); it!=keys.end(); it++) {
            qDebug() << "首层所有key： " << *it;
        }

        // 第一层数据遍历key,如code, message, sid
        for (int i=0; i<keys.size(); i++) {
            QString key = keys.at(i);
            QJsonValue value = obj.value(key);

            if (value.isBool()) {
                qDebug() << "1 Bool: " << key << ":" << value.toBool();
            } else if (value.isString()) {
                qDebug() << "1 String: " << key << ":" << value.toString();
            } else if (value.isDouble()) {
                qDebug() << "1 Double: " << key << ":" << value.toDouble();
            } else if (value.isObject()) {
                qDebug() << "1 Object: " << key;

                QJsonObject dataObj = value.toObject();

                QStringList dataKeys = dataObj.keys();

                QList<QString>::iterator it;
                for (it=dataKeys.begin(); it!=dataKeys.end(); it++) {
                    qDebug() << "第二层data所有key： " << *it;
                }

                // 第二层data遍历，如result, status
                for (int j=0; j<dataKeys.size(); j++) {
                    QString subKey = dataKeys.at(j);
                    QJsonValue subValue = dataObj.value(subKey);

                    if (subValue.isString()) {
                        qDebug() << "2 data String: " << subKey << ":" << subValue.toString();
                    } else if (subValue.isBool()) {
                        qDebug() << "2 data Bool: " << subKey << ":" << subValue.toBool();
                    } else if (subValue.isObject()) {
                        qDebug() << "2 data Object: " << subKey;

                        QJsonObject resultObj = subValue.toObject();
                        QStringList resultKeys = resultObj.keys();

                        QList<QString>::iterator it;
                        for (it=resultKeys.begin(); it!=resultKeys.end(); it++) {
                            qDebug() << "第三层result所有key： " << *it;
                        }

                        // 遍历第三层result的所有key，如bg, ed, ls, sn, ws
                        for (int k=0; k<resultKeys.size(); k++) {
                            QString thirdKey = resultKeys.at(k);
                            QJsonValue thirdValue = resultObj.value(thirdKey);

                            if (thirdValue.isString()) {
                                qDebug() << "3 result String: " << thirdKey << ":" << thirdValue.toString();
                            } else if (thirdValue.isBool()) {
                                qDebug() << "3 result Bool: " << thirdKey << ":" << thirdValue.toBool();
                            } else if (thirdValue.isArray()) {
                                qDebug() << "3 result Array: " << thirdKey;

                                QJsonArray wsArray = thirdValue.toArray();

                                for (int m=0; m<wsArray.size(); m++) {
                                    QJsonValue wsArrayValue = wsArray.at(m);

                                    switch (wsArrayValue.type()) {
                                    case QJsonValue::Bool:
                                        qDebug() << "4 ws Bool: " << wsArrayValue << wsArrayValue.toBool();
                                        break;
                                    case QJsonValue::Object:
                                    {
                                        qDebug() << "4 ws Object: " << wsArrayValue << wsArrayValue.toObject();

                                        QJsonObject wsObject = wsArrayValue.toObject();
                                        QStringList wsObjectKeys = wsObject.keys();

                                        QList<QString>::iterator it;
                                        for (it=wsObjectKeys.begin(); it!=wsObjectKeys.end(); it++) {
                                            qDebug() << "5 wsObjectKeys: " << *it;
                                        }

                                        for (int n = 0; n<wsObjectKeys.size(); ++n) {
                                            QString wsObjectKey = wsObjectKeys.at(n);
                                            QJsonValue wsObjectValue = wsObject.value(wsObjectKey);

                                            if (wsObjectValue.isBool()) {
                                                qDebug() << "5 wsObjectKey: " << wsObjectKey << wsObjectValue.toBool();
                                            } else if (wsObjectValue.isDouble()) {
                                                qDebug() << "5 wsObjectKey: " << wsObjectKey << wsObjectValue.toDouble();
                                            } else if (wsObjectValue.isString()) {
                                                qDebug() << "5 wsObjectKey: " << wsObjectKey << wsObjectValue.toString();
                                            } else if (wsObjectValue.isArray()) {
                                                qDebug() << "5 wsObjectKey Array: " <<wsObjectKey << wsObjectValue.toArray();

                                                QJsonArray cwArray = wsObjectValue.toArray();

                                                for (int l=0; l<cwArray.size(); l++) {
                                                    QJsonValue cwArrayValue = cwArray.at(l);

                                                    switch (cwArrayValue.type()) {
                                                    case QJsonValue::Object:
                                                    {
                                                        qDebug() << "6 cwArray Object: " << cwArrayValue.toObject();

                                                        QJsonObject cwArrayObject = cwArrayValue.toObject();
                                                        QStringList cwArrayObjectKeys = cwArrayObject.keys();

                                                        QList<QString>::iterator it;
                                                        for (it=cwArrayObjectKeys.begin(); it!=cwArrayObjectKeys.end(); it++) {
                                                            qDebug() << "6 cwArray key: " << *it;
                                                        }

                                                        for (int v = 0; v<cwArrayObjectKeys.size(); ++v) {
                                                            QString cwArrayObjectKey = cwArrayObjectKeys.at(v);
                                                            QJsonValue cwArrayObjectValue = cwArrayObject.value(cwArrayObjectKey);

                                                            if (cwArrayObjectValue.isString()) {
                                                                qDebug() <<"6 cwArray: " << cwArrayObjectKey << ":" << cwArrayObjectValue.toString();
                                                                resultText.append(cwArrayObjectValue.toString());
                                                            }
                                                        }
                                                    }
                                                        break;

                                                    default:
                                                        break;
                                                    }

                                                }
                                            }
                                        }
                                    }

                                        break;
                                    default:
                                        break;
                                    }

                                }

                            }
                        }


                    } else if (subValue.isArray()) {
                        QJsonArray subArray = subValue.toArray();

                        for (int k=0; k<subArray.size(); k++) {
                            if (subArray.at(k).isString()) {
                                qDebug() << subArray.at(k) << ":" << subArray.at(k).toString();
                            } else if (subArray.at(k).isBool()) {
                                qDebug() << subArray.at(k) << ":" << subArray.at(k).toBool();
                            }
                        }
                    }

                }
            }
        }

    }

    qDebug() << "resultText: " << resultText;
    emit parsetextMessageResultSignal(resultText);

}
#endif

