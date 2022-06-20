#include "widget.h"
#include "ui_widget.h"

#include <QDebug>

#include "voice/kvp_voicetransfer.h"
#include "kvp_actions.h"

static KVP_VoiceTransfer *kv = nullptr;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    kv = new KVP_VoiceTransfer;
    connect(kv, &KVP_VoiceTransfer::translateOk, [this](QString res){
        if (res.startsWith("打开")) {
            KVP_Actions ka;
            res.chop(1);
            res.remove(0, 2);
            qDebug() << res;
            QString z = "";
            QString act = "打开";
            ka.execAction(z, act, res);
        }
    });
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    if (kv)
        kv->testStart();
}

void Widget::on_pushButton_2_clicked()
{
    if (kv)
        kv->testEnd();
}
