#include "kvp_actions.h"

#include <QDebug>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>

#include "dbus/kvp_dbusadapter.h"

static void dbusFunc(const QString &_service,
                     const QString &_path,
                     const QString &_interface,
                     const QString &_function,
                     QVariantList &_list)
{
    KVP_DBusAdapter::callSessionFunction(_service, _path, _interface, _function, _list);
}

// 关机
static void sys_shutdown(QString &_intent, QString &_thing, QString &_enum, QString &_operator, QString &_property, QString &_joiner)
{
    char cmd[1024];
    memset(cmd, 0x00, 1024);
    if (_enum == "")
        sprintf(cmd, "shutdown -h now");
    else
        sprintf(cmd, "shutdown -h %s", _enum);
    system(cmd);
}

// 重启
static void sys_restart(QString &_intent, QString &_thing, QString &_enum, QString &_operator, QString &_property, QString &_joiner)
{
    char cmd[1024];
    memset(cmd, 0x00, 1024);
    sprintf(cmd, "shutdown -r %s", _enum);
    system(cmd);
}

// 锁屏
static void sys_lockscreen(QString &_intent, QString &_thing, QString &_enum, QString &_operator, QString &_property, QString &_joiner)
{
    QString service = "org.ukui.ScreenSaver",
            path = "/",
            interface = "org.ukui.ScreenSaver",
            func = "Lock";
    QVariantList vl;
    dbusFunc(service, path, interface, func, vl);
}

// 打开应用
static void openFunc(std::string _s)
{
    qDebug() << "exec open func " << QString::fromStdString(_s);
    QProcess::startDetached(QString::fromStdString(_s));
}

// 关闭应用
static void closeFunc(std::string _s)
{
    qDebug() << "exec close func " << QString::fromStdString(_s);
    QDBusMessage message = QDBusMessage::createSignal("/", "org.kylin.applications", "close");
    message << QString::fromStdString(_s);
    QDBusError err;
    if (KVP_DBusAdapter::sendSessionMessage(message, err) != 0)
        qDebug() << dbusErrorString(err);
}

KVP_Actions::KVP_Actions()
{
    actMap["打开"] = openFunc;
    actMap["播放"] = openFunc;
    actMap["关闭"] = closeFunc;
}

KVP_Actions::~KVP_Actions()
{

}

/**
 * 执行动作，通过主谓宾
 * @param [in]  : _subject 主语
 *                _predicate 谓语
 *                _object 宾语
 * @param [out] :
 * @return      :
 */
void KVP_Actions::execAction(QString &_subject, QString &_predicate, QString &_object)
{
//    QString st = "";
//    sys_lockscreen(st, st, st, st, st, st);
//    return;
//    if (_predicate == "功能") {
//        QString service = "org.ukui.kylinvideo",
//                path = "/org/ukui/kylinvideo",
//                interface = "org.ukui.kylinvideo.play",
//                func = "kylin_video_play_request";
//        QStringList sl;
//        sl << "/home/c/4K.mp4";
//        dbusFunc(service, path, interface, func, QVariantList() << sl);
//    }
//    return;

    for (auto& pair: appMap) {
        if (pair.first.find(_object.toStdString()) != pair.first.end()) {
            if (actMap.find(_predicate.toStdString()) != actMap.end()) {
                actMap[_predicate.toStdString()](pair.second);
            }
        }
    }
}

/**
 *
 * @param [in]  :   _intent 意图节点    (对应谓语)
 *                  _thing  对象节点    (对应宾语)
 *                  _enum   枚举节点    (属性结果值)
 *                  _operator   操作符节点
 *                  _property   属性边
 *                  _joiner     连接符
 * @param [out] :
 * @return      :
 */
void KVP_Actions::handleSemantics(QString &_intent, QString &_thing, QString &_enum, QString &_operator, QString &_property, QString &_joiner)
{

}
