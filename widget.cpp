#include "widget.h"
#include "ui_widget.h"
#include "voice/kvp_voicetransfer.h"
#include "kvp_actions.h"

#include <QDebug>

static KVP_VoiceTransfer *kv = nullptr;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    this->setFixedSize(QSize(400, 300));
    ui->setupUi(this);

    kv = new KVP_VoiceTransfer;

    connect(kv, &KVP_VoiceTransfer::translateOk, [this](QString res){
        qDebug() << "识别结果： " << res;

        if (res.startsWith("打开")) {
            KVP_Actions ka;
//            res.chop(1);
            res.remove(0, 2);
            qDebug() << "应用名称： " << res;

            QString z = "";
            QString act = "打开";
            ka.execAction(z, act, res);
        }
        if (res.startsWith("播放") || res.startsWith("音乐")) {
            KVP_Actions ka;
//            res.chop(1);
            res.remove(0, 2);
            qDebug() << "应用名称： " << res;

            QString z = "";
            QString act = "播放";
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
    if (kv) {
        kv->testStart();
    }
}

void Widget::on_pushButton_2_clicked()
{
    if (kv) {
        kv->testEnd();
    }
}
