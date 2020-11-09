#include "FtpTask.h"
#include <QDebug>

FtpTask::FtpTask(const LinkInfo &info, bool useThread, QObject *parent):
    QObject(parent),
    m_useThread(useThread),
    m_lastTransferDataSize(0)
{

    m_pTaskLink = new TaskExecutor(info);

    if(useThread)//使用线程
    {
        m_pThread = new QThread(this);
        m_pTaskLink->moveToThread(m_pThread);
        m_pThread->start();
    }
    else
        m_pThread = nullptr;

    //useThread为true时以下为线程函数,为false时则在主线程中执行
    connect(this, &FtpTask::dirChanged, m_pTaskLink, &TaskExecutor::on_dirChanged);
    connect(this, &FtpTask::listGot, m_pTaskLink, &TaskExecutor::on_listGot);
    connect(this, &FtpTask::fileSent, m_pTaskLink, &TaskExecutor::on_fileSent);
    connect(this, &FtpTask::dirSent, m_pTaskLink, &TaskExecutor::on_dirSent);
    connect(this, &FtpTask::filesGot, m_pTaskLink, &TaskExecutor::on_filesGot);
    connect(this, &FtpTask::dirGot, m_pTaskLink, &TaskExecutor::on_dirGot);
    connect(this, &FtpTask::dirDeleted, m_pTaskLink, &TaskExecutor::on_dirDeleted);
    connect(this, &FtpTask::filesDeleted, m_pTaskLink, &TaskExecutor::on_filesDeleted);
   // connect(this, &FtpTask::reTried, m_pTaskLink, &TaskExecutor::on_reTry);
    //处理完毕向外抛出done信号
    connect(m_pTaskLink, &TaskExecutor::done, [=](bool error, const QString &errMsg)
    {
        Q_EMIT done(error, errMsg);
    });
    connect(m_pTaskLink, &TaskExecutor::listInfo, [=](const QUrlInfo &info)
    {
        QUrlInfo localCodeInfo (info);
        QString name = info.name();
        localCodeInfo.setName(TaskExecutor::ftpToString(name));
        this->m_infoList.append(localCodeInfo);//保存
    });
    connect(m_pTaskLink,
            &TaskExecutor::dataTransferProgress,//数据传输处理，可以获得该信信号
            [=](qint64 sent, qint64 total)
    {
        qDebug() << __FILE__ << __LINE__ << "sum" << sent<< "size" << total;
        Q_EMIT dataTransferProgress(sent - m_lastTransferDataSize, total);
        m_lastTransferDataSize = sent;
    });
}

FtpTask::~FtpTask()
{
    stop();
}

void FtpTask::changeDir(const QString &targetDir)
{
    Q_EMIT dirChanged(targetDir);

}

void FtpTask::getList(const QString &objdirpath)
{
    Q_EMIT listGot(objdirpath);
}

void FtpTask::sendFiles(const QStringList &filepath, const QStringList &objdirpath, const QStringList &objfileName)
{
    Q_EMIT fileSent(filepath, objdirpath, objfileName);
}

void FtpTask::sendDir(const QString &filepath, const QString &objdirpath, const QString &objfileName)
{
    Q_EMIT dirSent(filepath, objdirpath, objfileName);
}

void FtpTask::getFiles(const QString &localPath, const QString &ftpPath, const QStringList &fileNames)
{
    Q_EMIT filesGot(localPath, ftpPath, fileNames);
}

void FtpTask::getDir(const QString &filepath, const QString &objdirpath, const QString &objfileName)
{
    Q_EMIT dirGot(filepath, objdirpath, objfileName);
}

void FtpTask::deleteDir(const QString &ftpDirPath, const QString &dirName)
{
    Q_EMIT dirDeleted(ftpDirPath, dirName);
}

void FtpTask::deleteFiles(const QStringList &ftpFilePaths)
{
    Q_EMIT filesDeleted(ftpFilePaths);
}

const QList<QUrlInfo> & FtpTask::infoList()
{
    return m_infoList;
}

//void FtpTask::reTry()
//{
//    Q_EMIT reTried();
//}

//QString ThreadTask::errMsg()
//{
//    return m_errMsg;
//}

void FtpTask::stop()
{
    qDebug() << __FILE__ << __LINE__ << __FUNCDNAME__;
    if(m_useThread && (nullptr != m_pThread))
    {
        m_pThread->requestInterruption();
        m_pThread->quit();
        m_pThread->wait(1);
    }
    if(nullptr != m_pTaskLink)
    {
        m_pTaskLink->deleteLater();
    }
}
