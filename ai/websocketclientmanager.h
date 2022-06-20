#ifndef WEBSOCKETCLIENTMANAGER_H
#define WEBSOCKETCLIENTMANAGER_H

#include <QQueue>
#include <QObject>
#include <QThread>
#include <QWebSocket>

class WebSocketClientManager : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClientManager(QObject *parent = nullptr);
    ~WebSocketClientManager();

public:
    bool running() const;

    QString url() const;
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
