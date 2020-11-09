#ifndef EXFTPCLIENT_H
#define EXFTPCLIENT_H

#include "FtpTask.h"
#include <QTimerEvent>
#include <QObject>

class FtpClientInterface : public QObject
{
    Q_OBJECT
public:

    explicit FtpClientInterface(const LinkInfo &info, QObject *parent = nullptr);

    //创建任务的所有入口
    FtpTask *changeDir(bool useThread, const QString &targetDir);//延根目录移动
    FtpTask *getList(bool useThread,  const QString &objdirpath);//list
    FtpTask *sendDir(bool useThread,  const QString &filepath, const QString &objdirpath, const QString &objfileName);
    FtpTask *sendFiles(bool useThread,  const QStringList &filepaths, const QStringList &objdirpaths, const QStringList &objfileNames);
    FtpTask *getFiles(bool useThread,  const QString &localPath, const QString &ftpFilePaths, const QStringList &fileNames);
    FtpTask *getDir(bool useThread,  const QString &localPath, const QString &ftpFilePath, const QString &fileName);
    FtpTask *deleteDir(bool useThread,  const QString &ftpDirPath, const QString &dirName);
    FtpTask *deleteFiles(bool useThread,  const QStringList &ftpFilePaths);
    //FtpTask * removeFile();
    //所有任务的出口
    void stopTask(FtpTask * target);//结束某个任务，并释放资源，如果打开了线程则立刻关闭线程
    // void reTry(FtpTask * target);//根据内置的信息重试任务

    //连接配置接口
    void changeLinkInfo(const LinkInfo &info);//改变要连接的服务器
    //获取目录树Item
    //QStandardItem * rootItem();
    //获取正在执行的任务总数
    qint32 runingTaskCount();
signals:
protected:
    void timerEvent(QTimerEvent *event) override;

private:
    int m_tid;
    LinkInfo m_linkInfo;//连接的信息，新建任务时
    //QStandardItem * m_pRoot;//存储目录数据和属性,通过信号和槽更新
    FtpTask *addTask(bool useThread);//创建一个任务
    QList<FtpTask *> m_taskList;//保存执行中的任务，从任务函数开始到服务器答复为一次完整的任务，届时释放任务对象
};

#endif // EXFTPCLIENT_H
