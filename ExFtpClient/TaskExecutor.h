#ifndef TASKLINK_H
#define TASKLINK_H

#include <QObject>
#include <qftp.h>
#include <QFile>

struct LinkInfo{
    QString host;
    quint16 port;
    QString userName;
    QString passWord;
};

class TaskExecutor: public QObject
{
    Q_OBJECT
public:
    enum TaskType{
        NONE,
        GETLIST,
        CDDIR,
        PUTFILE,
        PUTDIR,
        GETFILE,
        GETDIR,
        DELETEDIR,
        DELETEWAIT
    };//任务标号
    explicit TaskExecutor(const LinkInfo &info, QObject *parent = nullptr);
    ~TaskExecutor();
    void on_reTry();
    static QString ftpToString(QString &input);
    static QString stringToFtp(QString &input);
public slots:
    //所有任务的实现
    void on_getCurrentList();
    void on_dirChanged(QString targetDir);
    void on_listGot(QString dirpath);
    void on_fileSent(QStringList filepaths, QStringList objdirpaths, QStringList objfileNames);
    void on_dirSent(QString dirpath, QString objdirpath, QString objDirName);
    void on_filesGot(QString localPath, QString ftpPath, QStringList fileNames);
    void on_dirGot(QString localPath, QString objDirpath, QString dirName);
    void on_dirRemoved(QString objfilepath, QString objfileName);
    //所有对ftp返回信号的处理函数
    void on_dataTransferProgress(qint64 sum, qint64 size);
    void on_listInfo(const QUrlInfo & info);
    void on_ftpdone(bool error);
signals:
    void listInfo(const QUrlInfo & info);
    void dataTransferProgress(qint64 sum, qint64 size);
    void done(bool error,const QString & errMsg);//任务完成了,因为TaskLink不会马上释放所以采用引用传参
private:
    void ftpLinkStart();
    void ftpLinkClose();
    void putDir(QString dirPath, QString targetPath, QString subDirName);
    void putFile(const QString &localFilePath, const QString &ftpDirPath, const QString &fileName);
    void putFiles(const QStringList &filepathList,const QStringList &objdirpathList,const QStringList &objfileNameList);
    void getDir(const QString &localPath, const QString &ftpPath, const QString &dirName);
    void getFile(const QString &localPath, const QString &ftpPath, const QString &fileName);
    void getFiles(const QString &localPath, const QString &ftpFilePath, const QStringList &fileNames);

    LinkInfo m_lInfo;
    QFtp * m_pFtp;
    QString m_lastErrMsg;//错误信息
    TaskType m_taskType;//任务类型


    //下载目录时会访问的资源
    QString m_localPath;//本地文件路径
    QString m_ftpPath;//ftp文件路径
   QList<TaskExecutor *> m_downLoadDirTasks;
    // quint32 m_hasLoadUpCount;
    // QList<QFile *> m_opendFile;
    QList<QByteArray *> m_buffers;
private:
   void on_getDirDone(bool error,const QString & errMsg);
};

#endif // TASKLINK_H
