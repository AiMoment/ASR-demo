#include "widget.h"

#include <QApplication>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#include "kvp_actions.h"
#include "ai/websocketclientmanager.h"
#include "ai/aiengine.h"
#include "voice/kvp_voicetransfer.h"

#include <cstring>
#include <string.h>
#include <iostream>
using namespace::std;

//#include <ukui-log4qt.h>

int main(int argc, char *argv[])
{
//    initUkuiLog4qt("asr-demo");
    qSetMessagePattern("[ %{file}: %{line} ] %{message}");

    QApplication a(argc, argv);
    a.setOrganizationName("Kylin");
    a.setApplicationName("asr-demo");

    Widget w;
    w.show();

    return a.exec();
}
