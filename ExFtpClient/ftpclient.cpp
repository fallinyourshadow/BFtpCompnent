#include "ftpclient.h"
#include "runtimeconfig.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QIODevice>
#include <QDir>
#include <QTextCodec>
#include <QProgressBar>
#include <Windows.h>

FTPClient::FTPClient(QObject *parent)
{
    Q_UNUSED(parent)
    m_currentState = NONE;
    m_pBar = nullptr;
    m_taskNameList.clear();
    m_fileVector.clear();
    m_getFlag = true;
    m_getNum = 0;
    m_lastErrMsg.clear();
    //begin
    m_pFtp = nullptr;
    //end

}

FTPClient::~FTPClient()
{
    //begin
    login();
    //end

    m_pFtp->disconnect();
    m_pFtp->close();

    if (m_mutex.tryLock())
    {
        m_mutex.unlock();
    }
}

void FTPClient::sendFile(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p)
{
    m_filepath = filepath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    m_pBar = p;
    //begin
    login();
    //m_pFtp->connectToHost(m_server, FTP_PROT);
    //m_pFtp->login(m_username, m_password);
    //end
    m_currentState = PUT;
    m_runState = PUT;
    this->putFile(m_filepath, m_objpath, m_objName);//上传文件
}

void FTPClient::sendDirSlot(QString filepath, QString objfilepath, QString objfileName, QProgressBar *p)
{
    m_filepath = filepath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    m_pBar = p;
    //begin
    login();
    //m_pFtp->connectToHost(m_server, FTP_PROT);
    //m_pFtp->login(m_username, m_password);
    //end
    qDebug() << __FILE__ << __LINE__ << "Connected";

    m_currentState = PUTDIR;
    m_runState = PUTDIR;
    putDir(m_filepath, m_objName);
}

void FTPClient::recvFile(QString filepath, QString objfilepath,QString objfileName,QProgressBar *p)
{
    m_filepath = filepath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    m_pBar = p;
    //begin
    login();
//    m_pFtp->connectToHost(m_server, FTP_PROT);
//    m_pFtp->login(m_username, m_password);
    //end

    m_currentState = GET;
    m_runState = GET;
    this->getFile(m_filepath,m_objpath,m_objName);
}

void FTPClient::recvDir(QString dirpath, QString objfilepath, QString objfileName, QProgressBar *p)
{
    m_filepath = dirpath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    //begin
    login();
//    tmepFtp->connectToHost(m_server, FTP_PROT);
//    tmepFtp->login(m_username, m_password);
    //end
    this->getDir(dirpath,m_objpath,objfileName);
    m_pBar = p;
    m_currentState = GETDIR;
    m_runState = GETDIR;
}

void FTPClient::rmDir(QString objfilepath, QString objfileName)
{
    m_objpath = objfilepath;
    m_objName = objfileName;
    m_currentState = DELETEDIR;
    m_runState = DELETEDIR;
    //begin
    login();
    //    m_pFtp->connectToHost(m_server, FTP_PROT);
    //    m_pFtp->login(m_username, m_password);
    //end
    this->deleteDir(objfilepath,objfileName);//删除这个目录中的文件
}

void FTPClient::setServerSlot(QString server, const QString user, const QString password)
{
    m_server = server;
    m_username = user;
    m_password = password;
}

void FTPClient::timerEvent(QTimerEvent *event)
{

}

void FTPClient::slotDone(bool error)
{
    if(error)
    {
        //begin
        login();
        //end
        if(QFtp::Connected == m_pFtp->state())//如果处于连接状态
        {
            m_pFtp->disconnect();//断开连接
            m_pFtp->close();//关闭
        }
        qDebug() << __FILE__ << __LINE__ << "error";
        m_lastErrMsg = m_pFtp->errorString();//设置最后一次错误
        emit ftpDone(false, m_pBar);
        return;
    }

    switch (m_currentState) {
    case PUT:
    {
        qDebug() << __FILE__ << __LINE__ << "PUT";
        emit ftpDone(true, m_pBar);
        break;
    }
    case GET:
    {
        qDebug() << __FILE__ << __LINE__ << "GET";
        m_currentState = NONE;
        //begin
        emit ftpDone(m_lastErrMsg.isEmpty(), m_pBar);
//        if ( m_lastErrMsg.isEmpty())
//        {
//            emit ftpDone(true, m_pBar);
//        }
//        else
//        {
//            emit ftpDone(false, m_pBar);
//        }
        //end
        break;
    }
    case PUTDIR:
    {
        qDebug() << __FILE__ << __LINE__ << "PUTDIR";
        m_currentState = NONE;
        emit ftpDone(true, m_pBar);
        break;
    }

    case GETDIR:
    {
        qDebug() << __FILE__ << __LINE__ << "GETDIR";
        m_mutex.lock();
        if ( m_getFlag && m_getNum == 0)
        {
            m_currentState = NONE;
            emit ftpDone(true, m_pBar);
            emit getDoneSignal();
        }
        else
        {
            m_getFlag = true;
        }
        m_mutex.unlock();
        break;
    }
    case DELETEDIR:
    {
        qDebug() << __FILE__ << __LINE__ << "DELETEDIR";
        m_mutex.lock();
        //begin
        login();
        //end
        if (m_getFlag && m_getNum == 0) {
            m_pFtp->cd(m_objpath);
            m_pFtp->rmdir(m_objName);
            m_currentState = DELETEWAIT;
        }
        else {
            m_getFlag = true;
        }
        m_mutex.unlock();
        break;
    }
    case DELETEWAIT:
    {
        qDebug() << __FILE__ << __LINE__ << "DELETEWAIT";
        m_currentState = NONE;
        emit getDoneSignal();
        emit ftpDone(true, nullptr);
        break;
    }
    default:
        break;
    }

}

void FTPClient::ftpProgressSlot(qint64 sum, qint64 size)
{
    QString taskName;
    taskName.clear();
    if(m_taskNameList.isEmpty())
    {
        emit this->dataTransferProgressSignal(sum, size, m_pBar, taskName);
        return;
    }
    taskName = m_taskNameList.at(0);
    if (sum != size )
        return;
    if ( !m_taskNameList.isEmpty())
    {
        m_taskNameList.takeFirst();
    }
    if ( !m_fileVector.isEmpty())
    {
        QFile *file = m_fileVector.takeFirst();
        file->close();//关闭文件
        file->deleteLater();
        file = nullptr;
    }
}

void FTPClient::ListInfoSlot(const QUrlInfo &info)
{
    if (m_currentState == DELETEDIR)
    {
        if (info.isDir())//如果是目录
        {
            if ( info.name() == "." || info.name() == "..")//忽略的文件
                return;
            m_mutex.lock();
            m_getFlag = false;
            m_getNum++;
            m_mutex.unlock();

            FTPClient *client = new FTPClient(this);
            client->setServerSlot(m_server,m_username,m_password);
            connect(client,
                    &FTPClient::getDoneSignal,
                    this,
                    &FTPClient::getDoneSlot);
            connect(client,
                    &FTPClient::getDoneSignal,
                    client,
                    &FTPClient::deleteLater);
            //判断路径后面有没有 /符号
            if ( m_objpath == "/")
            {
                client->rmDir(m_objpath + m_objName,info.name());
            }
            else
            {
                client->rmDir(m_objpath + "/" + m_objName,info.name());
            }
        }
        else//如果是普通文件
        {
            //begin
            login();
            //end
            m_pFtp->remove(info.name());//删除
        }

        return;
    }
    QString name = info.name();
    QString dirPath = m_filepath + "/" + ftpToString(name);
    if ( info.isDir()) {
        if ( info.name() == "." || info.name() == "..") return;
        m_mutex.lock();
        m_getFlag = false;
        m_getNum++;
        m_mutex.unlock();
        FTPClient *client = new FTPClient(this);
        client->setServerSlot(m_server,m_username,m_password);
        connect(client,
                &FTPClient::dataTransferProgressSignal,
                this,
                &FTPClient::dataTransferProgressSignal);
        connect(client,
                &FTPClient::getDoneSignal,
                this,
                &FTPClient::getDoneSlot);
        connect(client,
                &FTPClient::getDoneSignal,
                client,
                &FTPClient::deleteLater);
        client->recvDir(dirPath, m_objpath, info.name(), m_pBar);
    }
    else
    {
        this->getFile(dirPath, m_objpath, info.name());
    }
}

void FTPClient::getDoneSlot()
{
    m_mutex.lock();
    m_getNum--;
    m_mutex.unlock();
    if ( m_getFlag && m_getNum == 0)
    {
        if ( m_currentState == DELETEDIR)
        {
            //begin
            login();
            //end
            m_currentState = DELETEWAIT;
            m_pFtp->cd(m_objpath);
            m_pFtp->rmdir(m_objName);
            return;
        }
        m_currentState = NONE;
        emit getDoneSignal();
        emit ftpDone(true,m_pBar);
    }
    else {
        m_mutex.lock();
        m_getFlag = true;
        //begin
        QThread * currentThread = QThread::currentThread();
        currentThread->requestInterruption();
        currentThread->wait(100);
        currentThread->quit();
        currentThread->deleteLater();
        //end
        m_mutex.unlock();
    }

}

void FTPClient::getFile(QString filePath,QString objPath,QString objName)
{
    QFile *file = new QFile(this);
    file->setFileName(filePath);
    if(!file->open(QIODevice::WriteOnly))
    {
        m_lastErrMsg = "Create file " + filePath + " failed.";
        return;
    }
    //begin
    login();
    //end

    m_pFtp->cd(objPath);
    m_pFtp->get(objName,file);
    m_taskNameList.append(file->fileName());
    m_fileVector.append(file);
}

void FTPClient::putFile(QString filePath, QString objPath, QString objName)
{
    QFile *file = new QFile(this);
    file->setFileName(filePath);//打开这个文件
    //begin
    login();
    //end
    if(!file->open(QIODevice::ReadOnly))//打开失败
    {
        m_pFtp->disconnect();
        m_pFtp->close();
        m_lastErrMsg = "Open file " + filePath + " failed.";
        return;
    }

    qDebug() << "out" << objName;
    m_pFtp->setTransferMode(QFtp::Passive);

    m_pFtp->cd(objPath.toUtf8());//相对于根的目录
    m_pFtp->put(file, objName, QFtp::Binary);//上传当前目录中的该文件

    m_taskNameList.append(file->fileName());//保存文件名
    m_fileVector.append(file);//保存文件的地址,收到done信号后关闭
    //Sleep(1);
}

void FTPClient::getDir(QString dirPath, QString objPath, QString objName)
{
    QDir dir("/");
    dir.mkpath(dirPath);//创建路径

    if ( m_objpath == "/")//根
    {
        m_objpath.append(objName);
    }
    else
    {
        m_objpath = objPath + "/" + objName;
    }
    //begin
    login();
    //end
    m_pFtp->cd(m_objpath);//移动到该路径下
    m_pFtp->list();//返回当前路径下的所有项目
}

void FTPClient::putDir(QString rootPath, QString currpath)
{
    QDir dir(currpath);
    if (!dir.exists())
    {
        return;
    }
    else
    {
        //begin
        login();
        //end
        m_pFtp->cd(stringToFtp(m_objpath));
        m_pFtp->mkdir(stringToFtp(currpath.split(rootPath).last()));
    }

    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();
    foreach (QFileInfo info, list) {
        if ( info.isDir()) {
            if(info.fileName()=="." || info.fileName() == "..")
                continue;
           // qDebug() << "output:" << info.absoluteFilePath();

            putDir(rootPath,info.absoluteFilePath());//递归
        }
        else {
            QString path = info.absoluteFilePath();
            QString ftpPath = m_objpath +"/" + currpath.split(rootPath).last();
            if ( m_objpath == "/") {
                ftpPath = m_objpath + currpath.split(rootPath).last();
            }
            QString fileName = info.fileName();
            //qDebug() << "output:" << info.fileName();
            putFile(path,stringToFtp(ftpPath),stringToFtp(fileName));
        }
    }
}

void FTPClient::deleteDir(QString objfilepath, QString objfileName)
{
    Q_UNUSED(objfilepath)
    //begin
    login();
    //end
    m_pFtp->cd(m_objpath);
    m_pFtp->cd(objfileName);
    m_pFtp->list();
}

bool FTPClient::login()
{
    if(m_pFtp != nullptr)//如果已经创建
        return false;
    m_pFtp = new QFtp(this);

    m_pFtp->connectToHost(m_server, FTP_PROT);
    m_pFtp->login(m_username, m_password);


    connect(m_pFtp,
            &QFtp::done,
            this,
            &FTPClient::slotDone);
    connect(m_pFtp,
            &QFtp::dataTransferProgress,//数据传输处理，应该可以获得文件的传输进度
            this,
            &FTPClient::ftpProgressSlot);
    connect(m_pFtp,
            &QFtp::listInfo,
            this,
            &FTPClient::ListInfoSlot);
    return true;
}

QString FTPClient::ftpToString(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return codec->toUnicode(input.toLatin1());
#else
    return input;
#endif
}

QString FTPClient::stringToFtp(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return QString::fromLatin1(codec->fromUnicode(input));
#else
    return input;
#endif
}
