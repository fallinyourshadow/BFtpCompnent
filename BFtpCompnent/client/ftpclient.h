﻿#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QThread>
#include <QFile>
#include "qftp.h"
#include <QMutex>
#include <QStringList>
#include <QVector>

class QProgressBar;
//enum{
//    NONE,
//    LOGIN,
//    PUT,
//    PUTDIR,
//    GET,
//    GETDIR,
//    DELETEDIR,
//    DELETEWAIT
//};
class FTPClient : public QObject
{
    Q_OBJECT
public:
    explicit FTPClient(QObject *parent = nullptr);
    ~FTPClient();

    //PUT
    void sendFile(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    //PUTDIR
    void sendDirSlot(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    //GET
    void recvFile(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    //GETDIR
    void recvDir(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    //DELETEDIR
    void rmDir(QString objfilepath, QString objfileName);

    void setServerSlot(QString server, const QString user , const QString password);

    QString getLastErrMsg() { return m_lastErrMsg;}
signals:

    void ftpDone(bool error,QProgressBar *);
    void dataTransferProgressSignal(qint64,qint64,QProgressBar *,QString);
    void getDoneSignal();
private slots:
    void slotDone(bool error);
    void ftpProgressSlot(qint64 sum, qint64 size);
    void ListInfoSlot(const QUrlInfo &info);
    void getDoneSlot();
private:
    QFtp * m_pFtp;
    enum{
        NONE,
        LOGIN,
        PUT,
        PUTDIR,
        GET,
        GETDIR,
        DELETEDIR,
        DELETEWAIT
    };
    QString m_server;
    QString m_filepath;
    QString m_objpath;
    QString m_objName;
    QString m_username;
    QString m_password;
    QProgressBar *m_pBar;

    bool m_getFlag;
    int m_getNum;

    int m_currentState;
    int m_runState;
    QString m_lastErrMsg;
    QMutex m_mutex;
    QStringList m_taskNameList;
    QVector<QFile *> m_fileVector;

    void getFile(QString filePath, QString objPath, QString objName);
    void putFile(QString filePath, QString objPath, QString objName);
    void getDir(QString dirPath,QString objPath,QString objName);
    void putDir(QString rootPath, QString currpath);
    void deleteDir(QString objfilepath, QString objfileName);
    bool login();
    QString ftpToString(QString &input);
    QString stringToFtp(QString &input);
};



#endif // FTPCLIENT_H
