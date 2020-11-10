#ifndef TASKLINK_H
#define TASKLINK_H

#include <QObject>
#include <qftp.h>
#include <QFile>
#include <QTimerEvent>
#include "defines.h"

#define  _EXTRA_TIMEOUT 1


class TaskExecutor: public QObject
{
    Q_OBJECT
public:
    explicit TaskExecutor(const LinkInfo &info, QObject *parent = nullptr);
    ~TaskExecutor();
    void on_reTry();
    static QString ftpToString(QString &input);
    static QString stringToFtp(QString &input);

public slots:
    //所有任务的实现
    void on_dirChanged(QString targetDir);
    void on_listGot(QString dirpath);
    void on_fileSent(QStringList filepaths, QStringList objdirpaths, QStringList objfileNames);
    void on_dirSent(QString dirpath, QString objdirpath, QString objDirName);
    void on_filesGot(QString localPath, QString ftpPath, QStringList fileNames);
    void on_dirGot(QString localPath, QString objDirpath, QString dirName);
    void on_dirDeleted(QString ftpDirPath, QString dirName);
    void on_filesDeleted(QStringList ftpFilePath);
    void on_nameChanged(QString ftpPath, QString oldName, QString newName);
    //所有对ftp返回信号的处理函数
    void on_dataTransferProgress(qint64 sum, qint64 size);
    void on_listInfo(const QUrlInfo & info);
    void on_ftpDone(bool error);
signals:
    void listInfo(const QUrlInfo & info);
    void dataTransferProgress(qint64 sum, qint64 size);
    void done(bool error,const QString & errMsg);//任务完成了,因为TaskLink不会马上释放所以采用引用传参
protected:
    explicit TaskExecutor(const LinkInfo &info, quint32 id, QObject *parent = nullptr);
    //申请外援(创建一个同类对象,用于递归调用,其隶属于同一个任务线程,用id来标识唯一性,这样就可以获得递归的层次)
    void timerEvent(QTimerEvent *event) override;
    TaskExecutor * applyExtraTaskExecutor(const LinkInfo &info, quint32 id, QObject *parent = nullptr);
protected slots:
    void on_extraTaskDone(bool error,const QString & errMsg);
private:
    void ftpLinkStart();
    void ftpLinkClose();
    void putDir(QString dirPath, QString targetPath, QString subDirName);
    void putFile(const QString &localFilePath, const QString &ftpDirPath, const QString &fileName);
    void putFiles(const QStringList &filepathList,const QStringList &objdirpathList,const QStringList &objfileNameList);
    void getDir(const QString &localPath, const QString &ftpPath, const QString &dirName);
    void getFile(const QString &localPath, const QString &ftpPath, const QString &fileName);
    void getFiles(const QString &localPath, const QString &ftpFilePath, const QStringList &fileNames);
    void deletFile(const QString &ftpFilePath);
    void deleteFiles(const QStringList &ftpFilePaths);
    void deleteDir(const QString &ftpDirPath,const QString &dirName);
    void deleteEmptyDir();
    void resetTimeout();
    LinkInfo m_lInfo;
    QFtp * m_pFtp;
    QString m_lastErrMsg;//错误信息
    TaskType m_taskType;//任务类型
    //上传文件会访问的资源
    quint32 m_hasUpLoadCount;
    // QList<QFile *> m_opendFile;
    QList<QByteArray *> m_buffers;
    QStringList m_putLocalPathList;//存储本地的路径
    QStringList m_putFtpPathList;//存储ftp路径
    QStringList m_putFileNameList;//存储文件名
    //下载或删除目录时会访问的资源
    QString m_localPath;//本地文件路径
    QString m_ftpPath;//ftp文件路径
    QStringList m_dirNames;
   // QStringList m_fileCount;
    QList<TaskExecutor *> m_processingDirTasks;
    quint32 m_extraId;//额外的任务id
    qint32 m_timeout;//用来检测额外的递归任务是否都完成了，如果完成的则向外层发送done信号
};

#endif // TASKLINK_H
