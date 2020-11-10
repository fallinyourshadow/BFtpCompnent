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
    m_lastErrMsg("success"),
    m_taskType(NONE),
    m_hasUpLoadCount(0),
    m_extraId(0),
    m_timeout(-1)
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
    case  PUTFILE:
        break;
    case  PUTDIR:
        break;
    case  GETFILE:
        break;
    case  GETDIR:
        break;
    case  DELETEDIR:
        break;
    default:
        break;
    }
}

void TaskExecutor::on_dirChanged(QString targetDir)
{
    ftpLinkStart();
    m_taskType = CDDIR;
    m_pFtp->cd(stringToFtp(targetDir));
}

void TaskExecutor::on_listGot(QString dirpath)
{
    ftpLinkStart();
    m_taskType = GETLIST;
    m_pFtp->list(stringToFtp(dirpath));
}

void TaskExecutor::on_fileSent(QStringList filepaths, QStringList objdirpaths, QStringList objfileNames)
{
    ftpLinkStart();
    m_taskType = PUTFILE;
    putFiles(filepaths, objdirpaths, objfileNames);
}

void TaskExecutor::on_dirSent(QString dirpath, QString objdirpath, QString objDirName)
{
    ftpLinkStart();
    //m_pFtp->setTransferMode(QFtp::Active);
    m_taskType = PUTDIR;
    putDir(dirpath,objdirpath,objDirName);
    m_hasUpLoadCount = 0;
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

void TaskExecutor::on_dirDeleted(QString ftpDirPath, QString dirName)
{
    ftpLinkStart();
    m_taskType = DELETEDIR;
    deleteDir(ftpDirPath, dirName);
}

void TaskExecutor::on_filesDeleted(QStringList ftpFilePath)
{
    ftpLinkStart();
    m_taskType = DELETEFILE;
    deleteFiles(ftpFilePath);
}

void TaskExecutor::on_nameChanged(QString ftpPath,QString oldName, QString newName)
{
    ftpLinkStart();
    m_taskType = CHANGENAME;
    QString old = ftpPath;
    old.append(oldName);
    QString nname = ftpPath;
    nname.append(newName);
    m_pFtp->rename(stringToFtp(old),stringToFtp(nname));
}

void TaskExecutor::on_dataTransferProgress(qint64 sum, qint64 size)
{
    if(sum == size && m_taskType == PUTDIR)
    {
        m_hasUpLoadCount++;
        if(!m_putLocalPathList.isEmpty())
            m_putLocalPathList.removeFirst();
        if(!m_putFtpPathList.isEmpty())
            m_putFtpPathList.removeFirst();
        if(!m_putFileNameList.isEmpty())
            m_putFileNameList.removeFirst();
        if(!m_buffers.isEmpty())
        {
            QByteArray * buf =
                    m_buffers.takeFirst();
            buf->clear();
            delete buf;
            buf = nullptr;
        }
    }
    Q_EMIT dataTransferProgress(sum, size);
    //qDebug() <<sum << size;
}

void TaskExecutor::on_listInfo(const QUrlInfo &info)
{

    QString name = info.name();
    if(name == "." || name == "..")//丢弃
        return;
    if(m_taskType == GETLIST)
    {
        Q_EMIT listInfo(info);
    }
    else if(m_taskType == GETDIR)
    {
        if(info.isDir())
        {
            //继续获取该目录下的文件信息
            TaskExecutor * getDirTask = applyExtraTaskExecutor(m_lInfo, m_extraId + 1, this);//创建一个外援
            getDirTask->on_dirGot(m_localPath, m_ftpPath, name);
        }
        else
        {
            //下载的文件
            getFile(m_localPath, m_ftpPath, name);
        }
    }
    else if(m_taskType == DELETEDIR)
    {
        // QString delname = info.name();
        if(info.isDir())
        {
            //继续获取该目录下的文件信息
            TaskExecutor *deleteDirTask = applyExtraTaskExecutor(m_lInfo, m_extraId + 1, this);//创建一个外援
            connect(deleteDirTask,&TaskExecutor::done,this,&TaskExecutor::on_extraTaskDone);
            m_processingDirTasks.append(deleteDirTask);
            deleteDirTask->on_dirDeleted(m_ftpPath, name);
            resetTimeout();
        }
        else
        {
            //删除目录中的文件
            QString filePath = m_ftpPath;
            filePath.append(name);
            deletFile(filePath);
            qDebug() << "dFile" << filePath;
        }
    }
    else
    {
        ;
    }
}

void TaskExecutor::on_ftpDone(bool error)
{
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
        qDebug() << __FILE__ << __LINE__ << m_processingDirTasks.size();
        if(m_processingDirTasks.size() == 0)//当当前下载任务正在下载的目录任务为0时，发送当前目录已经处理完毕
        {
            qDebug() << __FILE__ << __LINE__ << "GETDIR DONE";
            // Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        }
        else
        {
            qDebug() << __FILE__ << __LINE__ << "resetTimeout";
            resetTimeout();//设置超时检查
        }
        break;
    }
    case DELETEDIR:
    {
        qDebug() << __FILE__ << __LINE__ << m_processingDirTasks.size();
        if(m_processingDirTasks.size()  == 0)
        {
            qDebug() << __FILE__ << __LINE__ << "DELETEDIR DONE";
            //            if(m_extraId == 0)
            //            {
            //                //开始删除空目录
            //                qDebug() << "dsada"<< m_ftpPath;
            //                m_pFtp->rmdir(m_ftpPath);
            //            }
            //Q_EMIT done(error, m_lastErrMsg);
        }
        else
        {
            qDebug() << __FILE__ << __LINE__ << "resetTimeout";
            resetTimeout();//设置超时检查
        }
        break;
    }
    case DELETEFILE:
    {
        qDebug() << __FILE__ << __LINE__ << "DELETEFILE DONE";
        Q_EMIT done(error, m_lastErrMsg);//发送该次任务的执行情况
        break;
    }
    default:
        break;
    }
}

void TaskExecutor::timerEvent(QTimerEvent *event)
{
    if(event->timerId() != m_timeout)
        return;
    if(m_processingDirTasks.size() != 0)
        return;
    killTimer(m_timeout);
    qDebug() << __FILE__ << __LINE__;
    if(m_taskType == GETDIR)
    {
        Q_EMIT done(false,"download dir done");
    }
    else if(m_taskType == DELETEDIR)
    {
        if(m_processingDirTasks.size() == 0 )
        {
            if(m_extraId == 0)//回到第一层时
            {
                //开始删除空目录
                qDebug() << "dsada"<< m_ftpPath;
                m_pFtp->rmdir(m_ftpPath);
            }
            Q_EMIT done(false,"delete dir done");
        }
        else
        {
            resetTimeout();
        }
        //m_pFtp->rmdir(stringToFtp(m_ftpPath));
        //        foreach(QString dirName, m_dirNames)
        //        {
        //            QString path = m_ftpPath;
        //            path.append(dirName);
        //            qDebug() << __LINE__ << __FILE__ << path;
        //            m_pFtp->rmdir(path);
        //        }
        // Sleep(10000);
    }
    else
    {
        ;
    }


}

void TaskExecutor::ftpLinkStart()
{
    qDebug() << "link start";
    if(m_pFtp == nullptr)
        m_pFtp = new QFtp(this);
    if(m_pFtp->state() == QFtp::Unconnected)//如果处于未连接状态
    {
        m_pFtp->connectToHost(stringToFtp(m_lInfo.host), m_lInfo.port);
        m_pFtp->login(stringToFtp(m_lInfo.userName), stringToFtp(m_lInfo.passWord));
        //绑定所有可能执行任务后的结果处理槽函数
        connect(m_pFtp,&QFtp::done,this,&TaskExecutor::on_ftpDone);
        connect(m_pFtp,
                &QFtp::listInfo,
                this,
                &TaskExecutor::on_listInfo);//当获得listInfo时
        connect(m_pFtp,
                &QFtp::dataTransferProgress,//数据传输处理
                this,
                &::TaskExecutor::on_dataTransferProgress);
    }

    m_processingDirTasks.clear();
    //m_ftpPath.clear();
    //m_localPath.clear();
    m_buffers.clear();
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

void TaskExecutor::putDir(QString dirPath,QString targetPath,QString subDirName)
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsFirst);

    QFileInfoList list = dir.entryInfoList();//获取文件夹中所有的文件信息

    targetPath.append(subDirName).append("/");
    m_pFtp->mkdir(stringToFtp(targetPath));
    foreach (QFileInfo info, list)
    {
        if (info.isDir())
        {
            if(info.fileName() == "." || info.fileName() == "..")
                continue;
            qDebug() <<  "info.filePath()" << targetPath;
            putDir(info.filePath() ,targetPath, info.fileName());//递归
        }
        else
        {
            QString path = info.absoluteFilePath();//获得绝对路径
            QString fileName = info.fileName();
            QString ftpPath = targetPath;//在ftp上的路径
            qDebug() << "ftpPath" <<ftpPath;
            m_putLocalPathList.append(path);
            m_putFtpPathList.append(ftpPath);
            m_putFileNameList.append(fileName);
        }
    }
    putFiles(m_putLocalPathList, m_putFtpPathList, m_putFileNameList);//上传文件
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
            m_pFtp->put(*buf, stringToFtp(path.append(fileName)),QFtp::Binary);//直接操作绝对路径
        }
        else//如果不以/结尾
        {
            m_pFtp->put(*buf, stringToFtp(path.append("/").append(fileName)),QFtp::Binary);//直接操作绝对路径
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
    m_pFtp->cd(stringToFtp(ftpDirPath));//移动到该目录下
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
    qDebug() << "fileName" << fileName;
    m_pFtp->get(stringToFtp(ftpFilePath), file);//获取文件数据
}

void TaskExecutor::getFiles(const QString &localPath, const QString &ftpFilePath, const QStringList &fileNames)
{
    foreach(QString v,fileNames)
    {
        getFile(localPath,ftpFilePath,v);
    }
}

void TaskExecutor::deletFile(const QString &ftpFilePath)
{
    QString ftpstr = ftpFilePath;
    m_pFtp->remove(ftpstr);
    qDebug() << __FILE__ << __LINE__ << ftpstr;
}

void TaskExecutor::deleteFiles(const QStringList &ftpFilePaths)
{
    foreach(QString filePath,ftpFilePaths)
    {
        stringToFtp(filePath);
        deletFile(filePath);
    }
}

void TaskExecutor::deleteDir(const QString &ftpDirPath, const QString &dirName)
{
    m_ftpPath = ftpDirPath;
    QString name = dirName;
    m_ftpPath.append(name);
    if(m_extraId == 0)
    {
        m_ftpPath = stringToFtp(m_ftpPath);
    }
    m_dirNames.append(m_ftpPath);
    m_pFtp->list(m_ftpPath);//保存目录名，用于文件删除完后删除目录
    m_ftpPath.append("/");
}

void TaskExecutor::deleteEmptyDir()
{
    if(m_dirNames.size() > 0)
    {
        foreach(QString name, m_dirNames)
        {
            qDebug() << __LINE__ << name;
            m_pFtp->rmdir(name);
        }
    }
}

void TaskExecutor::resetTimeout()
{
    if(m_timeout != -1)
        killTimer(m_timeout);
    m_timeout = startTimer(_EXTRA_TIMEOUT);
}

void TaskExecutor::on_extraTaskDone(bool error, const QString &errMsg)
{
    //额外创建的任务完成了
    TaskExecutor * origin = qobject_cast< TaskExecutor *>(sender());
    origin->deleteEmptyDir();
    if(m_extraId == 0)
    {
        connect(origin,&TaskExecutor::done,[=](bool error,const QString &errMsg){

            qDebug() << "dsada"<< m_ftpPath;
            m_pFtp->rmdir(m_ftpPath);

            Q_EMIT done(error,errMsg);
        });
    }
    m_processingDirTasks.removeOne(origin);

//QString ftpPath = m_ftpPath;
//    if(m_taskType == DELETEDIR)
//    {
//        qDebug() << __LINE__ << __FILE__ << m_processingDirTasks.size();
//        foreach(QString dirName, m_dirNames)
//        {
//            QString path = m_ftpPath;
//            path.append(dirName);
//            qDebug() << __LINE__ << __FILE__ << path;
//            //m_pFtp->rmdir(path);
//        }
//    }
// origin->deleteLater();
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
    qDebug() << "unicode"<<codec->fromUnicode(input);
    qDebug() << "fromLatin1"<< QString::fromLatin1(codec->fromUnicode(input));
    return QString::fromLatin1(codec->fromUnicode(input));
#else
    return input;
#endif
}

TaskExecutor::TaskExecutor(const LinkInfo &info, quint32 id, QObject *parent):
    QObject(parent),
    m_lInfo(info),
    m_pFtp(nullptr),
    m_lastErrMsg("success"),
    m_taskType(NONE),
    m_hasUpLoadCount(0),
    m_extraId(id)
{

}

TaskExecutor *TaskExecutor::applyExtraTaskExecutor(const LinkInfo &info, quint32 id, QObject *parent)
{
    TaskExecutor * extra = new TaskExecutor(info, id, parent);
    m_processingDirTasks.append(extra);
    connect(extra, &TaskExecutor::done, this, &TaskExecutor::on_extraTaskDone);//额外的任务结束了
    return extra;
}
