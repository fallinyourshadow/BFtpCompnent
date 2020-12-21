#ifndef DEFINES_H
#define DEFINES_H
#include <QString>
#include <QTextCodec>
#include <QDebug>

struct LinkInfo{
    QString host;
    quint16 port;
    QString userName;
    QString passWord;
};


extern inline QString ftpToString(QString &input);

extern inline QString stringToFtp(QString &input);
#endif // DEFINES_H
