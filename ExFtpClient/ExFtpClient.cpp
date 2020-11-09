#include "ExFtpClient.h"
#include <QMessageBox>
#include <QDebug>
#include <stdio.h>

FtpClientInterface::FtpClientInterface(const LinkInfo &info, QObject *parent):QObject(parent),m_linkInfo(info)
{
    _setmaxstdio(0XFFF);
    m_tid = startTimer(5000);

    //m_pRoot = new QStandardItem;
}

FtpTask *FtpClientInterface::changeDir(bool useThread, const QString &targetDir)
{
    FtpTask * task = addTask(useThread);
    task->changeDir(targetDir);
    return task;
}

FtpTask *FtpClientInterface::getList(bool useThread,  const QString &objdirpath)
{
    FtpTask * task = addTask(useThread);
    task->getList(objdirpath);
    return task;
}

FtpTask *FtpClientInterface::sendDir(bool useThread, const QString &filepath, const QString &objdirpath, const QString &objfileName)
{
    FtpTask * task = addTask(useThread);
    task->sendDir(filepath, objdirpath, objfileName);
    return task;
}

FtpTask *FtpClientInterface::sendFiles(bool useThread, const QStringList &filepaths, const QStringList &objdirpaths, const QStringList &objfileNames)
{
    FtpTask * task = addTask(useThread);
    task->sendFiles(filepaths, objdirpaths, objfileNames);
    return task;
}

FtpTask *FtpClientInterface::getFiles(bool useThread, const QString &localPath, const QString &ftpFilePaths, const QStringList &fileNames)
{
    FtpTask * task = addTask(useThread);
    task->getFiles(localPath, ftpFilePaths, fileNames);
    return task;
}

FtpTask *FtpClientInterface::getDir(bool useThread, const QString &localPath, const QString &ftpFilePath, const QString &fileName)
{
    FtpTask * task = addTask(useThread);
    task->getDir(localPath, ftpFilePath, fileName);
    return task;
}

FtpTask *FtpClientInterface::deleteDir(bool useThread, const QString &ftpDirPath, const QString &dirName)
{
    FtpTask * task = addTask(useThread);
    task->deleteDir(ftpDirPath, dirName);
    return task;
}

FtpTask *FtpClientInterface::deleteFiles(bool useThread, const QStringList &ftpFilePaths)
{
    FtpTask * task = addTask(useThread);
    task->deleteFiles(ftpFilePaths);
    return task;
}


void FtpClientInterface::stopTask(FtpTask *target)
{
    //target->stop();
    qDebug() << __FILE__ << __LINE__ << __FUNCDNAME__;
    m_taskList.removeOne(target);//取出任务
    target->deleteLater();
}

//void FtpClientInterface::reTry(FtpTask *target)
//{
//    target->reTry();
//}



void FtpClientInterface::changeLinkInfo(const LinkInfo &info)
{
    m_linkInfo = info;
}

qint32 FtpClientInterface::runingTaskCount()
{
    return  m_taskList.size();
}

void FtpClientInterface::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_tid)
    {
        qDebug() << __FILE__ << __LINE__ <<  m_taskList.size();
    }
}

//QStandardItem *FtpClientInterface::rootItem()
//{
//    return m_pRoot;
//}

FtpTask *FtpClientInterface::addTask(bool useThread)
{
    FtpTask * task = new FtpTask(m_linkInfo, useThread, this);
    m_taskList.append(task);
    return task;
}



