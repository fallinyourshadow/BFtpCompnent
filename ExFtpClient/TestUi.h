#ifndef TESTUI_H
#define TESTUI_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class TestUi; }
QT_END_NAMESPACE
#include <QTreeView>
#include <QStandardItemModel>
#include "ExFtpClient.h"
#include <QMenu>

class TestUi : public QDialog
{
    Q_OBJECT

public:
    TestUi(QWidget *parent = nullptr);
    ~TestUi();

private slots:
    void on_cdDone(bool error, const QString &errMsg);
    void on_getListDone(bool error, const QString &errMsg);
    void on_upLoadDirDone(bool error, const QString &errMsg);
    void on_upLoadFileDone(bool error, const QString &errMsg);
    void on_downLoadFileDone(bool error, const QString &errMsg);
   void  on_downLoadDirDone(bool error, const QString &errMsg);
    void on_dataTransferProgress(qint64 sent, qint64 total);

    void on_pushButton_home_clicked();//home
    void on_treeView_doubleClicked(const QModelIndex &index);//双击
    void on_upLoadDirTriggered();//右键菜单上传
    void on_upLoadFileTriggered();//右键菜单上传
    void on_downLoadFileTriggered();//右键菜单下载
    void on_deleteFileTriggered();//右键菜单删除

    void on_treeView_pressed(const QModelIndex &index);

private:
    Ui::TestUi *ui;
    void changeDir(QString path);
    QStandardItemModel * m_pModel;
    FtpClientInterface * m_pExFtpClient;
    QMenu m_localFileOps;
    QMenu m_ftpFileOps;
    QList<QObject *> m_upLoadTaskList;
    qint64 m_sent;
    qint64 m_total;
    QList<qint32 > m_selectedRows;
};
#endif // TESTUI_H
