#ifndef KVP_DBUSADAPTER_H
#define KVP_DBUSADAPTER_H

#include <QObject>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusConnection>

#define dbusErrorString(err) QDBusError::errorString(err.type())

class KVP_DBusAdapter : public QObject
{
    Q_OBJECT
public:
    explicit KVP_DBusAdapter(QObject *parent = nullptr);
    ~KVP_DBusAdapter();

    static int sendSessionMessage(const QDBusMessage &_msg, QDBusError &_err);
    static int callSessionFunction(const QString &_service,
                                   const QString &_path,
                                   const QString &_interface,
                                   const QString &_function,
                                   const QVariantList &_arglist);

signals:

};

#endif // KVP_DBUSADAPTER_H
