#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QThread>
#include <QFile>
#include "qftp.h"
#include <QMutex>
#include <QStringList>
#include <QVector>

class QProgressBar;
class FTPClient : public QObject
{
    Q_OBJECT
public:
    explicit FTPClient(QObject *parent = 0);
    ~FTPClient();
    QFtp ftp;
    void sendFile(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    void sendDir(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    void recvFile(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    void recvDir(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p);
    void rmDir(QString objfilepath, QString objfileName);
    void setServer(QString server, const QString user , const QString password);
    QString getMsg() { return m_errMsg;}
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

    int currentState;
    int m_runState;
    QString m_errMsg;
    QMutex m_mutex;
    QStringList m_taskList;
    QVector<QFile *> m_fileVector;

    void getFile(QString filePath, QString objPath, QString objName);
    void putFile(QString filePath, QString objPath, QString objName);
    void getDir(QString dirPath,QString objPath,QString objName);
    void putDir(QString rootPath, QString currpath);
    void deleteDir(QString objfilepath, QString objfileName);
    QString ftpToString(QString &input);
    QString stringToFtp(QString &input);
};



#endif // FTPCLIENT_H
