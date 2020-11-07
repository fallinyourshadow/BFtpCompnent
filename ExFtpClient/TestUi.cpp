#include "TestUi.h"
#include "ui_TestUi.h"

#include <QHeaderView>
#include <QAction>
#include <QAbstractItemView>
#include "TestUi.h"
#include "TestUi.h"
#include "runtimeconfig.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QIODevice>
#include <QFileInfo>
//问题1 0kb的文件无法上传成功
//问题2 以“新建文件夹”命名的文件无法被正确识别
//问题3 下载目录线程结束条件未知，导致无法结束
//问题4 下载目录会出现崩溃的情况
//问t题5  下载中文命名的文件无法成功
//还未测试其上传下载文件大小的上限

TestUi::TestUi(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TestUi),
      m_sent (0),
      m_total (0)
{
    ui->setupUi(this);
    ui->progressBar->reset();

    ui->progressBar->setValue(100);
    m_pModel = new QStandardItemModel(this);
    ui->treeView->setModel(m_pModel);
    m_pModel->setHorizontalHeaderLabels(QStringList()
                                        << "fileName"
                                        << "修改日期"
                                        << "类型"
                                        << "限权"
                                        << "大小");
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);//可以多选
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);//右键菜单策略

    //创建客户端对象，理论上可以创建多个因为只有在任务中才会真正创教ftp连接不同的
    LinkInfo linkInfo = {SERVER_IP, FTP_PROT, USER_NAME, PASS_WORD};//默认的连接信息
    m_pExFtpClient = new FtpClientInterface(linkInfo,this);//创建交互接口

    //上传右键菜单
    QList<QAction *> list;
    //上传目录action
    QAction * upLoadDir = new QAction("上传目录",this);
    connect(upLoadDir, &QAction::triggered, this, &TestUi::on_upLoadDirTriggered);
    list << upLoadDir;

    //上传文件action
    QAction * upLoadFile = new QAction("上传文件",this);
    connect(upLoadFile, &QAction::triggered, this, &TestUi::on_upLoadFileTriggered);
    list << upLoadFile;
    //加入所有action
    m_localFileOps.addActions(list);

    //ftp文件操作菜单
    list.clear();//清空
    QAction * downLoadFile = new QAction("下载文件",this);
    connect(downLoadFile, &QAction::triggered, this, &TestUi::on_downLoadFileTriggered);
    list.append(downLoadFile);

    QAction * deleteFile = new QAction("删除文件",this);
    connect(deleteFile, &QAction::triggered, this, &TestUi::on_deleteFileTriggered);
    list.append(deleteFile);

    m_ftpFileOps.addActions(list);

    //绑定所有右键菜单到treeView
    connect(ui->treeView, &QTreeView::customContextMenuRequested,[=](const QPoint &pos){
        if(ui->treeView->selectedRows().size() > 0 )//如果选中了某个或多个item
        {
            m_ftpFileOps.move(cursor().pos());
            m_ftpFileOps.show();
        }
       if(!ui->treeView->indexAt(pos).isValid() )
        {
            m_localFileOps.move(cursor().pos());
            m_localFileOps.show();
        }
    });
}

TestUi::~TestUi()
{
    delete ui;
}


void TestUi::on_pushButton_home_clicked()
{
    //回到根目录
    ui->lineEdit_path->setText("/");
    changeDir("/");
}

void TestUi::on_cdDone(bool error, const QString &errMsg)
{
    FtpTask * origin = qobject_cast<FtpTask *>(sender()) ;
    if(error)
    {
        // //qDebug()<< __FILE__ << __LINE__ << errMsg;
        m_pExFtpClient->stopTask(origin);//停止或重试这个任务
        //m_pExFtpClient->reTry(origin);//重试，若选择重试就不要释放该对象，调用后会执行对应操作
    }
    else
    {
        m_pExFtpClient->stopTask(origin);//停止这个任务
    }
    origin->deleteLater();
    //qDebug() <<  __FILE__ << __LINE__ << errMsg;
}

void TestUi::on_getListDone(bool error, const QString &errMsg)
{
    FtpTask * origin = qobject_cast<FtpTask *>(sender()) ;
    if(error)
    {
        // //qDebug()<< __FILE__ << __LINE__ << errMsg;
        m_pExFtpClient->stopTask(origin);//停止这个任务
    }
    else
    {
        QList<QUrlInfo> infos =  origin->infoList();
        //qDebug() << __FILE__ << __LINE__ << infos.size();
        int i = 0;
        m_pModel->clear();
        m_pModel->setHorizontalHeaderLabels(QStringList()
                                            << "fileName"
                                            << "修改日期"
                                            << "类型"
                                            << "限权"
                                            << "大小");
        foreach(QUrlInfo v, infos)
        {
            QString name = v.name();
            m_pModel->setItem(i,0,new QStandardItem(TaskExecutor::ftpToString(name)));
            m_pModel->setItem(i,1,new QStandardItem(v.lastModified().toString()));
            if(v.isDir())
            {
                m_pModel->setItem(i,2,new QStandardItem("dir"));
            }
            else
            {
                QStringList list;
                list = v.name().split('.');
                if(list.size() > 1)
                {
                    m_pModel->setItem(i,2,new QStandardItem(list.last()));
                }
                else
                {
                    m_pModel->setItem(i,2,new QStandardItem(QString::fromLocal8Bit( "未知")));
                }
            }
            m_pModel->setItem(i,3,new QStandardItem(QString("%1").arg(v.permissions())));
            m_pModel->setItem(i,4,new QStandardItem(QString("%1%2").arg(v.size()).arg("bytes")));
            ++i;
            //qDebug() << __FILE__ << __LINE__ << v.name();
        }
        m_pExFtpClient->stopTask(origin);//停止这个任务
    }
}

void TestUi::on_upLoadDirDone(bool error, const QString &errMsg)
{

}

void TestUi::on_upLoadFileDone(bool error, const QString &errMsg)
{
    FtpTask * origin = qobject_cast<FtpTask *>(sender()) ;
    if(error)
    {
        // //qDebug()<< __FILE__ << __LINE__ << errMsg;
        m_upLoadTaskList.removeOne(origin);//移除一个任务标记
        m_pExFtpClient->stopTask(origin);//停止或重试这个任务
        //m_pExFtpClient->reTry(origin);//重试，若选择重试就不要释放该对象，调用后会执行对应操作
    }
    else
    {
        m_upLoadTaskList.removeOne(origin);//移除一个任务标记，用于共享进度条
        m_pExFtpClient->stopTask(origin);//停止这个任务
    }
    if(m_pExFtpClient->runingTaskCount() == 0)//当正在进行的任务数为0时清空计数
    {
        m_total = 0;
        m_sent = 0;
        m_upLoadTaskList.clear();
    }
    //qDebug() <<  __FILE__ << __LINE__ << errMsg;
}

void TestUi::on_downLoadFileDone(bool error, const QString &errMsg)
{
    FtpTask * origin = qobject_cast<FtpTask *>(sender()) ;
    if(error)
    {
        // //qDebug()<< __FILE__ << __LINE__ << errMsg;
        //m_upLoadTaskList.removeOne(origin);//移除一个任务标记
        m_pExFtpClient->stopTask(origin);//停止或重试这个任务
    }
    else
    {
        //m_upLoadTaskList.removeOne(origin);//移除一个任务标记，用于共享进度条
        m_pExFtpClient->stopTask(origin);//停止这个任务
    }
    //qDebug() <<  __FILE__ << __LINE__ << errMsg;
}

void TestUi::on_downLoadDirDone(bool error, const QString &errMsg)
{
    FtpTask * origin = qobject_cast<FtpTask *>(sender()) ;
    if(error)
    {
        // //qDebug()<< __FILE__ << __LINE__ << errMsg;
        //m_upLoadTaskList.removeOne(origin);//移除一个任务标记
        m_pExFtpClient->stopTask(origin);//停止或重试这个任务
    }
    else
    {
        //m_upLoadTaskList.removeOne(origin);//移除一个任务标记，用于共享进度条
        m_pExFtpClient->stopTask(origin);//停止这个任务
    }
}

void TestUi::on_dataTransferProgress(qint64 sent, qint64 total)
{
    QObject * Origin = sender();

    if(!m_upLoadTaskList.contains(Origin))
    {
        //qDebug() << __FILE__ << __LINE__<< "total" << total;
        m_upLoadTaskList.append(Origin);
        m_total += total;//进度条上限增加
    }
    m_sent += sent;//size总量增加
    int adder = m_sent/m_total * 100;
    // //qDebug() << __FILE__ << __LINE__<< "adder" << adder;
    //更新进度条
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(adder);
}

void TestUi::on_treeView_doubleClicked(const QModelIndex &index)
{
    QStandardItem * item = m_pModel->itemFromIndex(index);

    if(m_pModel->item(item->row(),2)->text() == "dir")
    {
        item = m_pModel->item(item->row(),0);
        //qDebug() << "item->text();"<< item->text();
        ui->lineEdit_path->setText(
                    ui->lineEdit_path->text().
                    append(item->text()).
                    append("/"));
        changeDir(ui->lineEdit_path->text());
    }
}

void TestUi::on_upLoadDirTriggered()
{

    QFileDialog dia;
    dia.setFileMode(QFileDialog::Directory);
    if(dia.exec()==QFileDialog::Rejected)
        return;
    QStringList list = dia.selectedFiles();
    ////qDebug() << list;
    if(m_pExFtpClient->runingTaskCount() == 0)
    {
        m_total = 0;
        m_sent = 0;
        ui->progressBar->reset();
        //m_uoLoadTaskList.clear();
    }
    foreach(QString v, list)
    {
        if(v == "." || v == "..")
            continue;
        if(!QFile(v).exists())
            continue;
        QFileInfo info(v);
        if(info.isDir())
        {
            FtpTask * sendDir =
                    m_pExFtpClient->sendDir(true,
                                            v,//目录本地绝对路径
                                            ui->lineEdit_path->text(),//ftp相对路径
                                            info.fileName());//目录名
            connect(sendDir,&FtpTask::done,this,&TestUi::on_upLoadDirDone);
            connect(sendDir,&FtpTask::dataTransferProgress,
                    this,&TestUi::on_dataTransferProgress);
        }
    }
}

void TestUi::on_upLoadFileTriggered()
{
    QFileDialog dia;
    dia.setFileMode(QFileDialog::ExistingFiles);
    if(dia.exec()==QFileDialog::Rejected)
        return;

    QStringList list = dia.selectedFiles();
    ////qDebug() << list;
    if(m_pExFtpClient->runingTaskCount() == 0)
    {
        m_total = 0;
        m_sent = 0;
        ui->progressBar->reset();
        //m_uoLoadTaskList.clear();
    }
    QStringList filepaths;
    QStringList objdirpaths;
    QStringList objfileNames;
    foreach(QString v, list)
    {
        if(v == "." || v == "..")
            continue;
        if(!QFile(v).exists())
            continue;
        QFileInfo info(v);
        if(info.isFile())
        {
            filepaths.append(v);
            objdirpaths.append(ui->lineEdit_path->text());
            objfileNames.append(info.fileName());
        }
    }
    FtpTask * putFile =
            m_pExFtpClient->sendFiles(true,filepaths,//本地绝对路径
                                      objdirpaths   ,//ftp路径
                                      objfileNames //文件名
                                      );
    connect(putFile,&FtpTask::done,this,&TestUi::on_upLoadFileDone);//传送完了
    connect(putFile,&FtpTask::dataTransferProgress,
            this,&TestUi::on_dataTransferProgress);//进度条参数
}

void TestUi::on_downLoadFileTriggered()
{
    //选择本地保存目录
    QFileDialog dia;
    dia.setFileMode(QFileDialog::Directory);
    if(dia.exec() == QFileDialog::Rejected)
        return;
    QString localPaths = dia.selectedFiles().at(0);//本地存放文件的地址
    QStringList ftpNamePaths;//文件名的地址
    QString path = ui->lineEdit_path->text();//获取当前ftp的操作目录
    QList<qint32> selectedRows = ui->treeView->selectedRows();
    foreach(qint32 row, selectedRows)//组成所有要下载的文件路径
    {
        QString name = m_pModel->item(row,0)->text();
        QString type = m_pModel->item(row,2)->text();
        if(type != "dir")
        {
            ftpNamePaths.append(name);
        }
        else
        {
            FtpTask * downLoadDir = m_pExFtpClient->getDir(true, localPaths, path, name);//下载目录
            connect(downLoadDir,&FtpTask::done,this,&TestUi::on_downLoadDirDone);
        }
    }
    if(ftpNamePaths.size() > 0)
    {
        FtpTask * downLoadFile = m_pExFtpClient->getFiles(true, localPaths, path, ftpNamePaths);//下载文件
        connect(downLoadFile,&FtpTask::done,this,&TestUi::on_downLoadFileDone);
    }
}

void TestUi::on_deleteFileTriggered()
{

}

void TestUi::changeDir(QString path)
{
    ui->lineEdit_path->setText(path);
    LinkInfo linkInfo = {SERVER_IP, FTP_PROT, USER_NAME, PASS_WORD};//默认的连接信息
    m_pExFtpClient->changeLinkInfo(linkInfo);//随时可以更改，因为只有在任务创建后才会使用
    //FtpTask * cdHome = m_pExFtpClient->changeDir(true,path);//
    FtpTask * getList = m_pExFtpClient->getList(true,path);//获得根目录中的文件列表
    //connect(cdHome,&FtpTask::done,this,&TestUi::on_cdDone);//cd操作完成
    connect(getList,&FtpTask::done,this,&TestUi::on_getListDone);//获取文件列表完成
}

void TestUi::on_treeView_pressed(const QModelIndex &index)
{
    m_selectedRows.clear();
    if(!m_selectedRows.contains(index.row()))
    {
        //qDebug() << index.row();
        m_selectedRows.append(index.row());
    }
}

