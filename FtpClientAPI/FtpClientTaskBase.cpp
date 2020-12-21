#include "FtpClientTaskBase.h"

FtpClientTaskBase::FtpClientTaskBase(QObject *parent) : QObject(parent)
{

}

void FtpClientTaskBase::onTaskStart(LinkInfo &linkInfo)
{
    qDebug() << "link start";
    if(m_pFtpClient == nullptr)
        m_pFtpClient = new QFtp(this);

    m_pFtpClient->connectToHost(stringToFtp(linkInfo.host), linkInfo.port);
    m_pFtpClient->login(stringToFtp(linkInfo.userName), stringToFtp(linkInfo.passWord));
        //绑定所有可能执行任务后的结果处理槽函数
    connect(m_pFtpClient,
            &QFtp::done,
            this,
            &FtpClientTaskBase::onDoneSignalReceived);
    connect(m_pFtpClient,
            &QFtp::listInfo,
            this,
            &FtpClientTaskBase::onListInfo);//当获得listInfo时
    connect(m_pFtpClient,
            &QFtp::dataTransferProgress,//数据传输处理
            this,
            &::FtpClientTaskBase::onDataTransferProgress);
}

void FtpClientTaskBase::onDoneSignalReceived(bool isError)
{
    if(isError)
    {
        m_errorInfos.append(m_pFtpClient->errorString());
    }
}

void FtpClientTaskBase::onDataTransferProgress(qint64 sum, qint64 size)
{
    m_processedPercent = (quint8)(size/sum);
    Q_EMIT processed(m_processedPercent);
}

void FtpClientTaskBase::onListInfo(const QUrlInfo &listInfo)
{
    m_listInfos.append(listInfo);
}
