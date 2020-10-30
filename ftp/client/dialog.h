#ifndef DIALOG_H
#define DIALOG_H

#include "qftp.h"
#include <QDialog>
#include <QString>
#include <QModelIndex>
#include <QMutex>

class QDragEnterEvent;
class FTPClient;
class QProgressBar;
class QFile;
class QUrlInfo;
class QDir;
class QPoint;
class QAction;
class QTreeWidgetItem;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void setFtpInfo(const QString &server,const QString &username,const QString &password,int port);

private:
    Ui::Dialog *ui;

    enum FileType {
        DIR = 0x10,
        OTHER
    };
    void connectToFtp();

    QFtp *m_pFtp;
    QAction *m_pActionDownload;
    QAction *m_pActionDelete;
    QAction *m_pActionRefresh;
    QAction *m_pActionOpen;
    QAction *m_pActionUpload;
    QAction *m_pActionRename;
    QAction *m_pActionNewDir;
    QAction *m_pActionUploadDir;

    QString m_server;
    QString m_username;
    QString m_password;
    int m_port;

    QString m_rootDir;
    QString m_currentDir;
    bool m_loginState;
    QFile *m_pFile;
    QString m_oldName;
    QTreeWidgetItem *m_oldItem;

    QString getFilePermissions(const QUrlInfo &urlInfo);
    QString ftpToString(QString &);
    QString stringToFtp(QString &);

    int m_threadNumber;
    QMutex m_threadMutex;

    void downloadDir(QString &savePath,QString &dirName);
    void downloadFile(QString &savePath,QString &fileName);

    void uploadFile(QStringList &fileList);
    void uploadDir(QString &dirPath);
private slots:
    void trigActionDownloadSlot();
    void trigActionDeleteSlot();
    void trigActionRefreshSlot();
    void trigActionOpenSlot();
    void trigActionUploadSlot();
    void trigActionUploadDirSlot();
    void trigActionRenameSlot();
    void renameSlot(QTreeWidgetItem*,int);
    void clickItemSlot(QTreeWidgetItem*item, int i);
    void triActionNewDirSlot();

    void doubleclickItemSlot(QTreeWidgetItem*,int);
    void treeWdtContexMenuSlot(QPoint);

    void ftpCommandStarted(int);
    void ftpCommandFinished(int,bool);
    void stateChangedSlot(int state);
    void ListInfoSlot(const QUrlInfo &info);

    void dataTransferProgressSlot(qint64, qint64, QProgressBar *, QString taskName);
    void threadDoneSlot(bool, QProgressBar *);

    void homeBtnClickedSlot();
    void lineDdieEnterSlot();
    void lineEditFindSlot(QString );
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    void sendState(const QString &);
};

#endif // DIALOG_H
