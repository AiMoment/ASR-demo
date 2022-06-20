#include "kvp_input_file.h"

#include <QDebug>

KVP_Input_File::KVP_Input_File()
{

}

KVP_Input_File::~KVP_Input_File()
{

}

int KVP_Input_File::open(QString &_file)
{
    // 当前只处理 pcm 文件
    srcFile = fopen(_file.toStdString().c_str(), "rb+");
    if (srcFile == nullptr) {
        qDebug() << "file open error!";
        return -1;
    }

    return 0;
}

int KVP_Input_File::read(char *_data, int _size, int _number)
{
    if (srcFile == nullptr)
        return -1;

    return fread(_data, _size, _number, srcFile);
}
