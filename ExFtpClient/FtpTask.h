#ifndef FTPTASK_H
#define FTPTASK_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "TaskExecutor.h"

class FtpTask: public QObject
{
    Q_OBJECT
signals:
    void dirChanged(QString targetDir);
    void listGot(QString objdirpath);
    void fileSent(QStringList filepaths, QStringList objdirpaths, QStringList objfileNames);
    void dirSent(QString dirpath, QString objdirpath, QString objDirName);
    void filesGot(QString localPath,QString ftpFilePath, QStringList fileNames);
    void dirGot(QString dirpath, QString objDirpath, QString objDirName);
    void dirDeleted(QString objfilepath, QString objfileName);
    void filesDeleted(QStringList ftpFilePaths);
    void reTried();
    //对外信号
    void done(bool error, const QString &errMsg);//任务结束了
    void dataTransferProgress(qint64 sum, qint64 deltaSize);//文件传输时的进度
public:
    explicit FtpTask(const LinkInfo &info, bool useThread = true, QObject *parent = nullptr);
    ~FtpTask();
    //通过接口启动线程函数或普通函数
    void changeDir(const QString &targetDir);
    void getList(const QString &objdirpath);
    void sendFiles(const QStringList &filepath,  const QStringList &objdirpath,  const QStringList &objfileName);
    void sendDir(const QString &filepath, const QString &objdirpath,  const QString &objfileName);
    void getFiles(const QString &localPath, const QString &ftpPath,  const QStringList &fileNames);
    void getDir(const QString &filepath, const QString &objdirpath,  const QString &objfileName);
    void deleteDir(const QString &ftpDirPath, const QString &dirName);
    void deleteFiles(const QStringList &ftpFilePaths);
    //成员变量操作
    const QList<QUrlInfo> &infoList();

    //void reTry();
    void stop();//停止线程，以及一些善后处理

public slots:

private:
    TaskExecutor * m_pTaskLink;//该需要完成的任务
    QThread * m_pThread;
    bool m_useThread;
    QList<QUrlInfo> m_infoList;
    qint64 m_lastTransferDataSize;
    // QString m_errMsg;
};

#endif // FTPTASK_H
