#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGSettings>
#include <KWindowSystem>
#include <QWidget>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QPainter>
#include <QStyle>
#include <QToolButton>
#include <QLabel>
#include <QSlider>
#include <QThread>
#include <QMenu>
#include <QStandardPaths>


#include <QTimer>
#include <QTime>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDesktopWidget>
#include <QtMultimedia>
#include <QAudioRecorder>
#include <QListWidget>

#include <QFile>
#include <QStackedLayout>
#include <QScrollBar>
#include <QMouseEvent>

#include <QPainter>
#include <QColor>
#include <QSettings>
#include <QKeyEvent>
#include <QRegExp>

#include <QDBusConnection>
#include <QDBusInterface>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <QDebug>


#include "asrthread.h"

#define KYLINRECORDER "org.asr-demo.settings"
#define FITTHEMEWINDOW "org.ukui.style"
#define FITCONTROLTRANS "org.ukui.control-center.personalise"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initGSettings();

    void paintEvent(QPaintEvent *); //毛玻璃绘制
    void transparencyChange(); // 透明度绘制

    static MainWindow *mutual; // 指针类型静态成员变量

    QGSettings *m_pathGSettings = nullptr; // 控制面板
    QGSettings *m_themeGSettings = nullptr; // 控制面板主题
    QGSettings *m_transparencyGSettings = nullptr; // 控制面板透明度

    double m_transparency = 0.60;  // 透明度
    QString fontType;
    int fontSize;
    int source;
    QString theme = "ukui-default";

    AsrThread *m_asrThread; // 录音线程

private slots:
    void on_toMemory_clicked(); // 开始录音
    void on_checkInputDevice_clicked(); // 检测录音设备
    void on_stopRecord_clicked(); // 结束录音

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
