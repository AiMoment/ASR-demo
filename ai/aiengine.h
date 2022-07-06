#ifndef KVP_AIENGINE_H
#define KVP_AIENGINE_H

#include <QObject>

#ifdef WEBSOCKET_AI
class WebSocketClientManager;
#endif


struct result_t {
    int err_no;
    QString sn;
    QString err_msg;
    QString type;
    QString result;
    uint64_t log_id;
    uint64_t start_time;
    uint64_t end_time;
};

class AIEngine : public QObject
{
    Q_OBJECT

public:
    explicit AIEngine(QObject *parent = nullptr);

    // 喂数据给它，处理后通过信号返回
    int feedData(char *_data, int _size);

    void enable();
    void disable();

signals:
    // 文字翻译
    void translateOk(QString);

    // 语法分析结构，目前只是主谓宾
    void analysisOk(QString, QString, QString);

private:

#ifdef WEBSOCKET_AI
    WebSocketClientManager *wscm;

    void initWebSocket();

    bool m_isEnable;

public slots:
    void textMessageReceivedParseSlot(QString str);
#endif

};

#endif // KVP_AIENGINE_H
