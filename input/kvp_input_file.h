#ifndef KVP_INPUT_FILE_H
#define KVP_INPUT_FILE_H

#include <QIODevice>

#include "kvp_input.h"

class KVP_Input_File : public KVP_Input
{
    Q_OBJECT
    enum FILE_TYPE {
        PCM,
        AAC
    };

public:
    KVP_Input_File();
    ~KVP_Input_File();

    int open(QString &_file);
    int read(char* _data, int _size, int _number);

private:
    FILE *srcFile;
};

#endif // KVP_INPUT_FILE_H
