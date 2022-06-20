#include "kvp_dbusadapter.h"
#include <QVector>

QDBusConnection sessionDBus = QDBusConnection::sessionBus();

KVP_DBusAdapter::KVP_DBusAdapter(QObject *parent) : QObject(parent)
{
}

KVP_DBusAdapter::~KVP_DBusAdapter()
{

}

int KVP_DBusAdapter::sendSessionMessage(const QDBusMessage &_msg, QDBusError &_err)
{
    if (sessionDBus.send(_msg) == false) {
        _err = sessionDBus.lastError();
        return -1;
    }
    return 0;
}

int KVP_DBusAdapter::callSessionFunction(const QString &_service,
                                         const QString &_path,
                                         const QString &_interface,
                                         const QString &_function,
                                         const QVariantList &_arglist)
{
    QDBusInterface interface(_service, _path, _interface);

    QVector<QVariant> vec_arg;
    for (int i=0; i<8; i++) {
        if (i < _arglist.size())
            vec_arg.append(_arglist.at(i));
        else
            vec_arg.append(QVariant());
    }

    interface.call(_function,
                   vec_arg.at(0),
                   vec_arg.at(1),
                   vec_arg.at(2),
                   vec_arg.at(3),
                   vec_arg.at(4),
                   vec_arg.at(5),
                   vec_arg.at(6),
                   vec_arg.at(7));
}
