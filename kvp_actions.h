#ifndef KVP_ACTIONS_H
#define KVP_ACTIONS_H

#include <QObject>
#include <QMap>
#include <map>
#include <set>
#include <iostream>
#include <functional>

using namespace std;

class KVP_Actions : public QObject
{
    Q_OBJECT
    enum TYPE{
        OPEN,
        CLOSE,
        DBUS
    };

    map<set<string>, string> appMap = {
        #include "app.inc"
    };
    map<string, function<void(string)>> actMap;

public:
    KVP_Actions();
    ~KVP_Actions();

    // 执行动作（主谓宾）
    void execAction(QString &_subject, QString &_predicate, QString &_object);

    // CMRL
    void handleSemantics(QString &_intent, QString &_thing, QString &_enum, QString &_operator, QString &_property, QString &_joiner);
};

#endif // KVP_ACTIONS_H
