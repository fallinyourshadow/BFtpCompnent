#include "TaskExecutor.h"
#include <QDebug>
#include <QThread>

#include <QDir>
#include <QTextCodec>
#include <windows.h>
inline bool hasDirPathEndChar(QString dirPath)
{
    return dirPath.endsWith("/") || dirPath.endsWith("\\");
}

TaskExecutor::TaskExecutor(const LinkInfo &info, QObject *parent):
    QObject(parent),
    m_lInfo(info),
    m_pFtp(nullptr),
    m_lastErrMsg("success")
  //, m_hasLoadUpCount(0)
{

}

TaskExecutor::~TaskExecutor()
{
    ftpLinkClose();
}

void TaskExecutor::on_reTry()
{
    switch (m_taskType) {
    case  NONE:
        break;
    case  GETLIST:
        break;
    case  CDDIR:
        break;
    case   PUTFILE:
        break;
    case  PUTDIR:
        break;
    case  GETFILE:
        break;
    case  GETDIR:
        break;
    case  DELETEDIR:
        break;
    case  DELETEWAIT:
        break;
    default:
        break;
    }
}

void TaskExecutor::on_getCurrentList()
{

}

void TaskExecutor::on_dirChanged(QString targetDir)
{
    qDebug() << __FUNCDNAME__ << QThread::currentThreadId() ;
    ftpLinkStart();
    m_taskType = CDDIR;
    m_pFtp->cd(targetDir);
}

void TaskExecutor::on_listGot(QString dirpath)
{
    ftpLinkStart();
    m_taskType = GETLIST;
    m_pFtp->list(dirpath);
}

void TaskExecutor::on_fileSent(QStringList filepaths, QStringList objdirpaths, QStringList objfileNames)
{
    ftpLinkStart();
    m_taskType = PUTFILE;
    //qDebug()  << "filepath"<<filepath << "objdirpath"<<objdirpath << "objfileName"<<objfileName;
    putFiles(filepaths, objdirpaths, objfileNames);
}

void TaskExecutor::on_dirSent(QString dirpath, QString objdirpath, QString objDirName)
{
    ftpLinkStart();
    //m_pFtp->setTransferMode(QFtp::Active);
    m_taskType = PUTDIR;
    putDir(dirpath,objdirpath,objDirName);
    // m_hasLoadUpCount = 0;
}

void TaskExecutor::on_filesGot(QString localPath,  QString ftpPath,  QStringList fileNames)
{
    ftpLinkStart();
    m_taskType = GETFILE;
    getFiles(localPath, ftpPath, fileNames);

}

void TaskExecutor::on_dirGot(QString localPath, QString ftpPath, QString dirName)
{
    ftpLinkStart();
    m_taskType = GETDIR;
    getDir(localPath,ftpPath,dirName);
}

void TaskExecutor::on_dirRemoved(QString objfilepath, QString objfileName)
{

}

void TaskExecutor::on_dataTransferProgress(qint64 sum, qint64 size)
{
    if(sum == size && m_taskType == PUTDIR)
    {
        //m_hasLoadUpCount++;
        //qDebug() << ;
        //        m_putLocalPathList.removeFirst();
        //        m_putFtpPathList.removeFirst();
        //        m_putFileNameList.removeFirst();

        //        QByteArray * buf =
        //        m_buffers.takeFirst();
        //        buf->clear();
        //        delete buf;
        //        buf = nullptr;
    }
    Q_EMIT dataTransferProgress(sum, size);
    qDebug() <<sum << size;
}

void TaskExecutor::on_listInfo(const QUrlInfo &info)
{

    if(m_taskType == GETDIR)
    {
        QString ftpPath = m_ftpPath;
        QString localPath = m_localPath;
       // ftpPath.append("/").append(info.name());
       // localPath.append("/").append(info.name());
        if(info.isDir())
        {
            //获取该目录下的信息
            TaskExecutor   * getDirTask = new TaskExecutor(m_lInfo);
            connect(getDirTask,&TaskExecutor::done,this,&TaskExecutor::on_getDirDone);
            m_downLoadDirTasks.append(getDirTask);
            getDirTask->on_dirGot(localPath,ftpPath, info.name());
        }
        else if(info.isFile())
        {
            //下载的文件
            qDebug() << "getFile";
            getFile(localPath, ftpPath, info.name());
        }
    }
    if(m_taskType == GETLIST)
    {
        Q_EMIT listInfo(info);
    }

    //    /qDebug() << info.name();
}

void TaskExecutor::on_ftpdone(bool error)
{
//    if(error)//如果发生了错误
//    {
//        //        if(m_taskType == PUTDIR)//重新发送
//        //        {
//        //           putFiles(m_putLocalPathList,m_putFtpPathList,m_putFileNameList);
//        //        }
//        m_lastErrMsg = m_pFtp->errorString();//读出最后一次错误
//        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
//        return;
//    }
    switch (m_taskType) {
    case GETLIST:
    {
        qDebug() << __FILE__ << __LINE__ << "GETLIST DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    case CDDIR:
    {
        qDebug() << __FILE__ << __LINE__ << "CDDIR DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    case PUTFILE:
    {
        qDebug() << __FILE__ << __LINE__ << "PUT DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    case GETFILE:
    {
        qDebug() << __FILE__ << __LINE__ << "GET DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    case PUTDIR:
    {

        qDebug() << __FILE__ << __LINE__ << "PUTDIR DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    case GETDIR:
    {
        qDebug() << __FILE__ << __LINE__ << m_downLoadDirTasks.size();
        if(0 == m_downLoadDirTasks.size())//当当前下载任务正在下载的目录任务为0时，发送当前目录已经处理完毕
        {
            qDebug() << __FILE__ << __LINE__ << "GETDIR DONE";
            Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        }
        else
        {

        }
        break;
    }
    case DELETEDIR:
    {
        qDebug() << __FILE__ << __LINE__ << "DELETEDIR DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    case DELETEWAIT:
    {
        qDebug() << __FILE__ << __LINE__ << "DELETEWAIT DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    default:
        break;
    }

}


void TaskExecutor::ftpLinkStart()
{
    qDebug() << "link start";
    m_pFtp = new QFtp(this);
    if(m_pFtp->state() == QFtp::Unconnected)//如果处于未连接状态
    {
        m_pFtp->connectToHost(m_lInfo.host, m_lInfo.port);
        m_pFtp->login(m_lInfo.userName, m_lInfo.passWord);
        //绑定所有可能执行任务后的结果处理槽函数
        connect(m_pFtp,&QFtp::done,this,&TaskExecutor::on_ftpdone);
        connect(m_pFtp,
                &QFtp::listInfo,
                this,
                &TaskExecutor::on_listInfo);//当获得listInfo时
        connect(m_pFtp,
                &QFtp::dataTransferProgress,//数据传输处理，应该可以获得文件的传输进度
                this,
                &::TaskExecutor::on_dataTransferProgress);
    }
}

void TaskExecutor::ftpLinkClose()
{
    if(nullptr == m_pFtp)
        return;
    if(m_pFtp->state() == QFtp::LoggedIn||
            m_pFtp->state() == QFtp::Connected)//如果已经登陆或连接
    {
        m_pFtp->disconnect();
    }
    m_pFtp->close();
    qDebug() << "task relase";
}

void TaskExecutor::putDir(QString dirPath,QString targetPath, QString subDirName)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsFirst);
    QStringList putLocalPathList;//存储本地的路径
    QStringList putFtpPathList;//存储ftp路径
    QStringList putFileNameList;//存储文件名
    QFileInfoList list = dir.entryInfoList();//获取文件夹中所有的文件信息

    targetPath.append(subDirName).append("/");
    m_pFtp->mkdir(stringToFtp(targetPath));
    foreach (QFileInfo info, list)
    {
        if (info.isDir())
        {
            if(info.fileName() == "." || info.fileName() == "..")
                continue;
            qDebug() <<  info.filePath() << targetPath;
            putDir(info.filePath() ,targetPath, info.fileName());//递归
        }
        else
        {
            QString path = info.absoluteFilePath();//获得绝对路径
            QString fileName = info.fileName();
            QString ftpPath = targetPath;//将要在ftp上的路径
            putLocalPathList.append(path);
            putFtpPathList.append(ftpPath);
            putFileNameList.append(fileName);
        }
    }
    putFiles(putLocalPathList, putFtpPathList, putFileNameList);//上传文件
}

void TaskExecutor::putFile(const QString &localFilePath, const QString &ftpDirPath, const QString &fileName)
{
    QFile * file = new QFile(localFilePath);
    if(!file->open(QIODevice::ReadOnly))
    {
        Q_EMIT done(true,QString::fromLocal8Bit("open %1 failed").arg(localFilePath));//文件打开失败
        return;
    }
    QByteArray * buf = new QByteArray(file->readAll());//读出
    if(!buf->isNull())
    {
        m_buffers.append(buf);//保存缓存区
        file->close();//关闭
        delete file;//删除
        //m_pFtp->cd(objdirpath);
        QString path = ftpDirPath;
        if(hasDirPathEndChar(ftpDirPath))//如果以/结尾
        {
            m_pFtp->put(*buf, path.append(fileName),QFtp::Binary);//直接操作绝对路径
        }
        else//如果不以/结尾
        {
            m_pFtp->put(*buf, path.append("/").append(fileName),QFtp::Binary);//直接操作绝对路径
        }
    }
}

void TaskExecutor::putFiles(const QStringList &filepathList, const QStringList &objdirpathList, const QStringList &objfileNameList)
{
    foreach(QString v, filepathList)
    {
        int i = filepathList.indexOf(v);
        putFile(v, objdirpathList.at(i), objfileNameList.at(i));
    }
}

void TaskExecutor::getDir(const QString &localPath, const QString &ftpPath, const QString &dirName)
{
    QDir dir;
    QString localDirPath = localPath;
    localDirPath.append("/").append(dirName);
    QString ftpDirPath = ftpPath;
    ftpDirPath.append(dirName);
    m_localPath = localDirPath;
    m_ftpPath = ftpDirPath;
    //创建本地的目录
    dir.mkpath(localDirPath);
    qDebug() << "localDirPath" << localDirPath;
    qDebug() << "ftpDirPath" << ftpDirPath;
    qDebug() << " info.name())" << dirName;
    m_pFtp->cd(ftpDirPath);//移动到该目录下
    m_ftpPath.append("/");
    m_pFtp->list();//返回当前路径下的所有项目
}

void TaskExecutor::getFile(const QString &localPath, const QString &ftpPath, const QString &fileName)
{

    QFile *file = new QFile(this);
    QString localFilePath;
    QString ftpFilePath;
    localFilePath.append(localPath).append("/").append(fileName);//组建本地路径
    file->setFileName(localFilePath);//文件名
    if(file->exists())
        return;//文件已存在
    if(!file->open(QIODevice::WriteOnly))//创建，只能写入
        return;
    ftpFilePath.append(ftpPath).append(fileName);//组建ftp路径
    qDebug() << "localFilePath" << localFilePath;
    qDebug() << "ftpDirPath" << ftpFilePath;
    qDebug() << " fileName" << fileName;
     m_pFtp->get(ftpFilePath,file);//获取文件数据
}

void TaskExecutor::getFiles(const QString &localPath, const QString &ftpFilePath, const QStringList &fileNames)
{
    foreach(QString v,fileNames)
    {
        getFile(localPath,ftpFilePath,v);
    }
}

void TaskExecutor::on_getDirDone(bool error, const QString &errMsg)
{
    TaskExecutor * origin = qobject_cast< TaskExecutor *>(sender());
    m_downLoadDirTasks.removeOne(origin);
    origin->deleteLater();
}

QString TaskExecutor::ftpToString(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return codec->toUnicode(input.toLatin1());
#else
    return input;
#endif
}

QString TaskExecutor::stringToFtp(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return QString::fromLatin1(codec->fromUnicode(input));
#else
    return input;
#endif
}
