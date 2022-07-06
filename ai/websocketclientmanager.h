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
 * WetSocket API文档： https://www.xfyun.cn/doc/asr/voicedictation/API.html#%E6%8E%A5%E5%8F%A3%E8%B0%83%E7%94%A8%E6%B5%81%E7%A8%8B
 *
 * 定义部分
 *
 * 语音听写，所涉及参数的定义
 * 用于wss通信类iat_client的定义
 ***************************************************
 */

/// 接口鉴权参数，服务接口认证信息
/// 在讯飞开放平台控制台，创建WebAPI平台应用并添加语音听写（流式版）服务后即可查看，均为32位字符串
typedef struct API_IFNO
{
    string APISecret; // 获取接口密钥认证信息APIKey
    string APIKey; // 获取接口认证信息APISecret。
}API_INFO;

// 公共参数
typedef struct COMMON_INFO
{
    string APPID; // 在平台申请的APPID信息
} COMMON_INFO;

// 业务参数
typedef struct BUSINESS_INFO
{
    string language; // 语种， zh_cn：中文（支持简单的英文识别） en_us：英文
    string domain; // 应用领域， iat：日常用语 medical：医疗
    string accent; // 方言，当前仅在language为中文时，支持方言选择。 mandarin：中文普通话、其他语种
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
    string encoding; // 音频数据格式，raw：原生音频（支持单声道的pcm） speex：speex压缩后的音频（8k） speex-wb：speex压缩后的音频（16k）
    // string audio; // audio程序运行中指定
} DATA_INFO;

// 其他参数
typedef struct OTHER_INFO
{
    string audio_file; // 需识别的音频文件，这里采用麦克风实时识别，因此可以为空
} OTHER_INFO;

 /***************************************************/
 /***************************************************/


class WebSocketClientManager : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClientManager(QObject *parent = nullptr);
    ~WebSocketClientManager();

public:
    void initSocket();
    bool isRunning() const;

    string getAuthorizeUrl();
    QString getUrl() const;
    void setUrl(const QString &url);
    void connectUrl(QString url);

    void startDistinguish();
    void endDistinguish();

    QString makeFirstFrameText(QString data);
    QString makeContinueFrameText(const QByteArray &data);
    QString makeLastFrameText(QString data);

    void sendBinaryMessage(char* data, int len);


signals:
    void connectedSignal();
    void disconnectedSignal();

    void sendTextMessageResultSignal(bool result);
    void sendBinaryMessageResultSignal(bool result);

    void errorSignal(QString errorString);

    void textFrameReceivedSignal(QString frame, bool isLastFrame);
    void textMessageReceivedSignal(QString message);

    void sendContinueFrameAudioDataSignal(QString continueFrame); // 发送组装后的音频数据
    void sendDirectAudioDataSignal(QByteArray data); // 直接发送音频数据

public slots:
    void connectedSlot();
    void disconnectedSlot();
    void stopSlot();

    void sendTextMessageSlot(const QString &message);
    void sendBinaryMessageSlot(const QByteArray &data);

    void errorSlot(QAbstractSocket::SocketError error);
    void sslErrorSlot(const QList<QSslError> &errors);

    void textMessageReceivedSlot(const QString &message);
    void textFrameReceivedSlot(const QString &frame, bool isLastFrame);

private:
    API_IFNO m_apiInfo; // 接口授权参数
    COMMON_INFO m_commonInfo; // 公共参数
    BUSINESS_INFO m_businessInfo;  // 业务参数
    DATA_INFO m_dataInfo; // 数据流参数
    OTHER_INFO m_otherInfo; // 其他参数

    bool m_isRunning; // 当前 websocket 是否正在运行
    bool m_isConnected;
    bool m_isEnabled;
    bool m_isStarted;

    QString m_authorizeUrl; // 授权认证URL
    QWebSocket *m_pWebSocket; // websocket 连接句柄
    QQueue<QByteArray> m_audioDataQueue; // 从麦克风获取的音频数据队列
};

#endif // WEBSOCKETCLIENTMANAGER_H
