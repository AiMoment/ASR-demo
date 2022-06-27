#ifndef WEBSOCKETCLIENTMANAGER_H
#define WEBSOCKETCLIENTMANAGER_H

#include <QObject>
#include <QQueue>
#include <QThread>
#include <QWebSocket>
#include <QtNetwork/QSslError>

#include "iflytek_utils.h"
#include <string.h>
#include <iostream>

using namespace  std;


/***************************************************
 * 定义部分
 *
 * 语音听写，所涉及参数的定义
 * 用于wss通信类iat_client的定义
 ***************************************************
 */
// 接口鉴权参数
typedef struct API_IFNO
{
    string APISecret;
    string APIKey;
}API_INFO;

// 公共参数
typedef struct COMMON_INFO
{
    string APPID;
} COMMON_INFO;

// 业务参数
typedef struct BUSINESS_INFO
{
    string language;
    string domain;
    string accent;
    // 更多个性化参数可在官网查看
} BUSINESS_INFO;

// 帧标识，标识音频是第一帧，还是中间帧、最后一帧
enum STATUS_INFO
{
    STATUS_FIRST_FRAME = 0,    // 第一帧的标识
    STATUS_CONTINUE_FRAME, // 中间帧标识
    STATUS_LAST_FRAME,     // 最后一帧的标识
};

// 业务数据流参数
typedef struct DATA_INFO
{
    // int status; // status程序运行中指定
    string format;
    string encoding;
    // string audio; // audio程序运行中指定
} DATA_INFO;

// 其他参数
typedef struct OTHER_INFO
{
    string audio_file;
} OTHER_INFO;



class WebSocketClientManager : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClientManager(QObject *parent = nullptr);
    ~WebSocketClientManager();

public:
    bool running() const;

    API_IFNO API;
    COMMON_INFO COMMON;
    BUSINESS_INFO BUSINESS;
    DATA_INFO DATA;
    OTHER_INFO OTHER;
    QString url() const;
    string get_url();
    void setUrl(const QString &url);

    void connectUrl(QString url);
    void startDistinguish();
    void endDistinguish();
    void sendBinaryMessage(char* data, int len);

signals:
    void signal_connected();
    void signal_disconnected();
    void signal_sendTextMessageResult(bool result);
    void signal_sendBinaryMessageResult(bool result);
    void signal_error(QString errorString);
    void signal_textFrameReceived(QString frame, bool isLastFrame);
    void signal_textMessageReceived(QString message);

    void sig_send(QByteArray data);

public slots:
    void slot_stop();
    void slot_sendTextMessage(const QString &message);
    void slot_sendBinaryMessage(const QByteArray &data);

protected slots:
    void slot_connected();
    void slot_disconnected();
    void slot_error(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);
    void slot_textFrameReceived(const QString &frame, bool isLastFrame);
    void slot_textMessageReceived(const QString &message);

private:
    void initSocket();

    bool _running;
    bool _connected;
    bool enable;
    bool start;
    QString _url;
    QWebSocket *_pWebSocket;
    QQueue<QByteArray> _dataQueue;
};

#endif // WEBSOCKETCLIENTMANAGER_H
