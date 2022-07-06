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


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.show();

    return a.exec();
}
