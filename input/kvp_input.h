#ifndef KVP_INPUT_H
#define KVP_INPUT_H

#include <QObject>

class KVP_Input : public QObject
{
    Q_OBJECT
//    enum IO_TYPE{
//        MIC,
//        FILE,
//        SYS
//    };

public:
    KVP_Input(QObject *parent = nullptr);
    ~KVP_Input();

    void set_nb_samples(int _nb_samples) {nb_samples = _nb_samples;}
    virtual int open(QString&) = 0;

signals:
    void dataReady(char **data, int _nb_samples);

private:
    int nb_samples;
};

#endif // KVP_INPUT_H
