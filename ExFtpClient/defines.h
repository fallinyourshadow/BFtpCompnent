#ifndef DEFINES_H
#define DEFINES_H
#include <QObject>

struct LinkInfo{
    QString host;
    quint16 port;
    QString userName;
    QString passWord;
};

enum TaskType{
    NONE,
    GETLIST,
    CDDIR,
    PUTFILE,
    PUTDIR,
    GETFILE,
    GETDIR,
    DELETEDIR,
    DELETEWAIE,//删除目录需要等待所有文件删除完后再删除所有空目录
    DELETEFILE,
    CHANGENAME
};//任务标号
#endif // DEFINES_H
