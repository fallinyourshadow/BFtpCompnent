#include "ftpclient.h"
#include "../runtimeconfig.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QIODevice>
#include <QDir>
#include <QTextCodec>

//#define FTP_PROT 21

FTPClient::FTPClient(QObject *parent)
{
    Q_UNUSED(parent);
    currentState = NONE;
    m_pBar = NULL;
    m_taskList.clear();
    m_fileVector.clear();
    m_getFlag = true;
    m_getNum = 0;
    m_errMsg.clear();

    connect(&ftp, SIGNAL(done(bool)),
            this, SLOT(slotDone(bool)));
    connect(&ftp,SIGNAL(dataTransferProgress(qint64,qint64)),this,
            SLOT(ftpProgressSlot(qint64,qint64)));
    connect(&ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(ListInfoSlot(QUrlInfo)));
}

FTPClient::~FTPClient()
{
    ftp.disconnect();
    ftp.close();
    if ( m_mutex.tryLock()) {
        m_mutex.unlock();
    }
}

void FTPClient::sendFile(QString filepath, QString objfilepath, QString objfileName,QProgressBar *p)
{
    m_filepath = filepath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    m_pBar = p;
    ftp.connectToHost(m_server, FTP_PROT);
    ftp.login(m_username, m_password);
    currentState = PUT;
    m_runState = PUT;
    this->putFile(m_filepath,m_objpath,m_objName);
}

void FTPClient::sendDir(QString filepath, QString objfilepath, QString objfileName, QProgressBar *p)
{
    m_filepath = filepath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    m_pBar = p;
    ftp.connectToHost(m_server, FTP_PROT);
    ftp.login(m_username, m_password);

    currentState = PUTDIR;
    m_runState = PUTDIR;

    putDir(m_filepath,m_objName);
}

void FTPClient::recvFile(QString filepath, QString objfilepath,QString objfileName,QProgressBar *p)
{
    m_filepath = filepath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    m_pBar = p;
    ftp.connectToHost(m_server, FTP_PROT);
    ftp.login(m_username, m_password);
    currentState = GET;
    m_runState = GET;
    this->getFile(m_filepath,m_objpath,m_objName);
}

void FTPClient::recvDir(QString dirpath, QString objfilepath, QString objfileName, QProgressBar *p)
{
    m_filepath = dirpath;
    m_objpath = objfilepath;
    m_objName = objfileName;
    ftp.connectToHost(m_server, FTP_PROT);
    ftp.login(m_username, m_password);
    this->getDir(dirpath,m_objpath,objfileName);
    m_pBar = p;
    currentState = GETDIR;
    m_runState = GETDIR;
}

void FTPClient::rmDir(QString objfilepath, QString objfileName)
{
    m_objpath = objfilepath;
    m_objName = objfileName;
    currentState = DELETEDIR;
    m_runState = DELETEDIR;
    ftp.connectToHost(m_server, FTP_PROT);
    ftp.login(m_username, m_password);
    this->deleteDir(objfilepath,objfileName);
}

void FTPClient::setServer(QString server, const QString user, const QString password)
{
    m_server = server;
    m_username = user;
    m_password = password;
}

void FTPClient::slotDone(bool error)
{
    if(error)
    {
        if(QFtp::Connected == ftp.state())
        {
            ftp.disconnect();
            ftp.close();
        }
       m_errMsg = ftp.errorString();
       emit ftpDone(false, m_pBar);
       return;
    }
    switch (currentState) {
    case PUT:
    case GET:
        currentState = NONE;
        if ( m_errMsg.isEmpty()) {
            emit ftpDone(true, m_pBar);
        }
        else {
            emit ftpDone(false, m_pBar);
        }
        break;
    case PUTDIR:
        currentState = NONE;
        emit ftpDone(true, m_pBar);
        break;
    case GETDIR:
        m_mutex.lock();
        if ( m_getFlag && m_getNum == 0) {
            currentState = NONE;
            emit ftpDone(true, m_pBar);
            emit getDoneSignal();
        }
        else {
            m_getFlag = true;
        }
        m_mutex.unlock();
        break;
    case DELETEDIR:
        m_mutex.lock();
        if ( m_getFlag && m_getNum == 0) {
            ftp.cd(m_objpath);
            ftp.rmdir(m_objName);
            currentState = DELETEWAIT;
        }
        else {
            m_getFlag = true;
        }
        m_mutex.unlock();
        break;
    case DELETEWAIT:
        currentState = NONE;
        emit getDoneSignal();
        emit ftpDone(true, NULL);
        break;
    default:
        break;
    }
}

void FTPClient::ftpProgressSlot(qint64 sum, qint64 size)
{
    QString taskName;
    taskName.clear();
    if ( !m_taskList.isEmpty()) {
        taskName = m_taskList.at(0);
    }

    emit this->dataTransferProgressSignal(sum,size,m_pBar,taskName);
    if ( sum == size ) {
        if ( !m_taskList.isEmpty()) {
            m_taskList.takeFirst();
        }
        if ( !m_fileVector.isEmpty()) {
            QFile *file = m_fileVector.takeFirst();
            file->close();
            file->deleteLater();
            file = NULL;
        }
    }
}

void FTPClient::ListInfoSlot(const QUrlInfo &info)
{
    if ( currentState == DELETEDIR) {
        if ( info.isDir()) {
            if ( info.name() == "." || info.name() == "..") return;
            m_mutex.lock();
            m_getFlag = false;
            m_getNum++;
            m_mutex.unlock();

            FTPClient *client = new FTPClient(this);
            client->setServer(m_server,m_username,m_password);
            connect(client,SIGNAL(getDoneSignal()),this,SLOT(getDoneSlot()));
            connect(client,SIGNAL(getDoneSignal()),client,SLOT(deleteLater()));
            if ( m_objpath == "/") {
                client->rmDir(m_objpath + m_objName,info.name());
            }
            else {
                client->rmDir(m_objpath + "/" + m_objName,info.name());
            }

        }
        else {
            ftp.remove(info.name());
        }

        return;
    }

    QString dirPath = m_filepath + "/" + ftpToString(info.name());
    if ( info.isDir()) {
        if ( info.name() == "." || info.name() == "..") return;
        m_mutex.lock();
        m_getFlag = false;
        m_getNum++;
        m_mutex.unlock();
        FTPClient *client = new FTPClient(this);
        client->setServer(m_server,m_username,m_password);
        connect(client,SIGNAL(dataTransferProgressSignal(qint64,qint64,QProgressBar*,QString)),
                this,SIGNAL(dataTransferProgressSignal(qint64,qint64,QProgressBar*,QString)));
        connect(client,SIGNAL(getDoneSignal()),this,SLOT(getDoneSlot()));
        connect(client,SIGNAL(getDoneSignal()),client,SLOT(deleteLater()));
        client->recvDir(dirPath,m_objpath,info.name(),m_pBar);
    }
    else {
        this->getFile(dirPath,m_objpath,info.name());
    }
}

void FTPClient::getDoneSlot()
{
    m_mutex.lock();
    m_getNum--;
    m_mutex.unlock();
    if ( m_getFlag && m_getNum == 0) {
        if ( currentState == DELETEDIR) {
            currentState = DELETEWAIT;
            ftp.cd(m_objpath);
            ftp.rmdir(m_objName);
            return;
        }
        currentState = NONE;
        emit getDoneSignal();
        emit ftpDone(true,m_pBar);
    }
    else {
        m_mutex.lock();
        m_getFlag = true;
        m_mutex.unlock();
    }
}

void FTPClient::getFile(QString filePath,QString objPath,QString objName)
{
    QFile *file = new QFile(this);
    file->setFileName(filePath);
    if(!file->open(QIODevice::WriteOnly))
    {
        m_errMsg = "Create file " + filePath + " failed.";
        return;
    }
    ftp.cd(objPath);
    ftp.get(objName,file);
    m_taskList.append(file->fileName());
    m_fileVector.append(file);
}

void FTPClient::putFile(QString filePath, QString objPath, QString objName)
{
    QFile *file = new QFile(this);
    file->setFileName(filePath);
    if(!file->open(QIODevice::ReadOnly))
    {
        ftp.disconnect();
        ftp.close();
        m_errMsg = "Open file " + filePath + " failed.";
        return;
    }
    ftp.cd(objPath);
    ftp.put(file, objName);
    m_taskList.append(file->fileName());
    m_fileVector.append(file);
}

void FTPClient::getDir(QString dirPath, QString objPath, QString objName)
{
    QDir dir("/");
    dir.mkpath(dirPath);

    if ( m_objpath == "/") {
        m_objpath.append(objName);
    }
    else {
        m_objpath = objPath + "/" + objName;
    }
    ftp.cd(m_objpath);
    ftp.list();
}

void FTPClient::putDir(QString rootPath, QString currpath)
{
    QDir dir(currpath);
    if ( !dir.exists()) {
        return;
    }
    else {
        ftp.cd(stringToFtp(m_objpath));
        ftp.mkdir(stringToFtp(currpath.split(rootPath).last()));
    }

    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsLast);
    QFileInfoList list = dir.entryInfoList();

    foreach (QFileInfo info, list) {
        if ( info.isDir()) {
            if(info.fileName()=="." || info.fileName() == "..") continue;
            putDir(rootPath,info.absoluteFilePath());
        }
        else {
            QString path = info.absoluteFilePath();
            QString ftpPath = m_objpath +"/" + currpath.split(rootPath).last();
            if ( m_objpath == "/") {
                ftpPath = m_objpath + currpath.split(rootPath).last();
            }
            putFile(path,stringToFtp(ftpPath),stringToFtp(info.fileName()));
        }
    }
}

void FTPClient::deleteDir(QString objfilepath, QString objfileName)
{
    ftp.cd(m_objpath);
    ftp.cd(objfileName);
    ftp.list();
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
