#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *MainWindow::mutual = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    mutual = this;

    m_asrThread = new AsrThread();

    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    initGSettings();

    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initGSettings()
{
    if (QGSettings::isSchemaInstalled(KYLINRECORDER)) {
        m_pathGSettings = new QGSettings(KYLINRECORDER);
    }
    if (m_pathGSettings != nullptr) {

        connect(m_pathGSettings, &QGSettings::changed, this, [=]() {
            source = m_pathGSettings->get("source").toInt();
        });
    }

    if (QGSettings::isSchemaInstalled(FITTHEMEWINDOW)) {
        m_themeGSettings = new QGSettings(FITTHEMEWINDOW);
    }
    if (m_themeGSettings != nullptr) {
        connect(m_themeGSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                theme = m_themeGSettings->get("styleName").toString();
//                themeStyle(theme);
            } else if (key == "systemFontSize") {
                fontType = m_themeGSettings->get("systemFont").toString();
                fontSize = m_themeGSettings->get("systemFontSize").toInt();
//                themeFontStyle(fontType, fontSize);
            }
        });
//        themeStyle(m_themeGSettings->get("styleName").toString());
//        themeFontStyle(m_themeGSettings->get("systemFont").toString(), m_themeGSettings->get("system-font-size").toInt());
    }

    if (QGSettings::isSchemaInstalled(FITCONTROLTRANS)) {
        m_transparencyGSettings = new QGSettings(FITCONTROLTRANS);
    }

    if (m_transparencyGSettings != nullptr) {
        connect(m_transparencyGSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "transparency") {
                transparencyChange();
            }
        });
        transparencyChange();
    }

}


void MainWindow::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);

    QColor color = palette().color(QPalette::Window);

    color.setAlpha(m_transparency);

    QPalette pal(this->palette());
    pal.setColor(QPalette::Window,QColor(color));
    this->setPalette(pal);
    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

}

void MainWindow::transparencyChange()
{
    m_transparency = m_transparencyGSettings->get("transparency").toDouble() * 255;

    this->update();
}



void MainWindow::on_toMemory_clicked()
{
    m_asrThread->startRecord();
}


void MainWindow::on_checkInputDevice_clicked()
{
    QAudioDeviceInfo inputDevice(QAudioDeviceInfo::defaultInputDevice());
    source = m_pathGSettings->get("source").toInt();

    qDebug() << "inputDevice.deviceName = " << inputDevice.deviceName();

    if (inputDevice.deviceName().contains("monitor")) {
        qDebug() << "No input device detected.";
        QMessageBox::warning(MainWindow::mutual, tr("Warning"), tr("No input device detected!"), QMessageBox::Ok);
    }

}

void MainWindow::on_stopRecord_clicked()
{
    m_asrThread->stopRecord();
}
