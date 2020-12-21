#ifndef FTPCLIENTBASE_H
#define FTPCLIENTBASE_H

#include "qftp.h"
#include "defines.h"

class FtpClientTaskBase : public QObject
{
    Q_OBJECT
public:
    explicit FtpClientTaskBase(QObject *parent = nullptr);
signals:
    void taskDone(const QStringList &errorInfos);
    void processed(const qint8 &percent);
public slots:
    virtual void onTaskStart(LinkInfo &linkInfo);
    virtual void onDoneSignalReceived(bool isError);
    virtual void onDataTransferProgress(qint64 sum, qint64 size);
    virtual void onListInfo(const QUrlInfo& listInfo);
private:
    QFtp *m_pFtpClient;
    QStringList m_errorInfos;
    quint8 m_processedPercent;
    QList<QUrlInfo> m_listInfo;
};

#endif // FTPCLIENTBASE_H
