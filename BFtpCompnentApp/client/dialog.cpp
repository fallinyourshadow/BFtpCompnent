#include "dialog.h"
#include "ui_dialog.h"
#include "ftpclient.h"
//#include "../runtimeconfig.h"

#include <QTreeWidgetItem>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QDirModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QModelIndex>

#include <QProgressBar>
#include <QDateTime>
#include <QFileIconProvider>
#include <QTextCodec>
#include <QApplication>

#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QTimer>
#include <QUrl>

//#define FTP_PROT 21

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);


    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    this->setWindowTitle(QStringLiteral("云盘管理"));
    //this->resize(900,500);
    this->resize(687,363);

    m_rootDir = "/";
    m_currentDir = "/";
    ui->lineEditPath->setText(m_currentDir);
    ui->treeWidgetFtp->setAcceptDrops(true);

    m_oldItem = nullptr;
    m_threadNumber = 0;
    m_oldName.clear();
    m_loginState = false;

    m_pFtp = new QFtp;
    connect(m_pFtp,
            &QFtp::stateChanged,
            this,
            &Dialog::stateChangedSlot);
    connect(m_pFtp,
            &QFtp::listInfo,
            this,
            &Dialog::ListInfoSlot);
    connect(m_pFtp,
            &QFtp::commandStarted,
            this,
            &Dialog::ftpCommandStarted);
    connect(m_pFtp,
            &QFtp::commandFinished,
            this,
            &Dialog::ftpCommandFinished);

    ui->treeWidgetFtp->setRootIsDecorated(false);
    ui->treeWidgetFtp->setSortingEnabled(true);
    ui->treeWidgetFtp->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeWidgetFtp->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->scrollArea->setVisible(false);

    ui->lineEdit->setVisible(false);

    m_pActionDelete = new QAction(this);
    m_pActionDelete->setText(QStringLiteral("删除(&D)"));
    QIcon deleteIcon(":/img/ftp/stop.ico");
    //m_pActionDelete->setIcon(deleteIcon);
    m_pActionDelete->setToolTip(QStringLiteral("delete"));
    connect(m_pActionDelete,
            &QAction::triggered,
            this,
            &Dialog::trigActionDeleteSlot);

    m_pActionDownload =new QAction(this);
    m_pActionDownload->setText(QStringLiteral("下载(&A)"));
    QIcon downloadIcon(":/img/ftp/down.ico");
    //m_pActionDownload->setIcon(downloadIcon);
    connect(m_pActionDownload,
            &QAction::triggered,
            this,
            &Dialog::trigActionDownloadSlot);

    m_pActionOpen = new QAction(this);
    m_pActionOpen->setText(QStringLiteral("打开(&O)"));
    QIcon openIcon(":/img/ftp/open.ico");
    //m_pActionOpen->setIcon(openIcon);
    connect(m_pActionOpen,
            &QAction::triggered,
            this,
            &Dialog::trigActionOpenSlot);

    m_pActionRefresh = new QAction(this);
    m_pActionRefresh->setText(QStringLiteral("刷新(&R)"));
    QIcon refreshIcon(":/img/ftp/sync.ico");
    //m_pActionRefresh->setIcon(refreshIcon);
    connect(m_pActionRefresh,
            &QAction::triggered,
            this,
            &Dialog::trigActionRefreshSlot);

    m_pActionUpload = new QAction(this);
    m_pActionUpload->setText(QStringLiteral("上传(&U)"));
    QIcon uploadIcon(":/img/ftp/up.ico");
    //m_pActionUpload->setIcon(uploadIcon);
    connect(m_pActionUpload,
            &QAction::triggered,
            this,
            &Dialog::trigActionUploadSlot);

    m_pActionUploadDir = new QAction(this);
    m_pActionUploadDir->setText(QStringLiteral("上传文件夹(&Y)"));
    connect(m_pActionUploadDir,
            &QAction::triggered,
            this,
            &Dialog::trigActionUploadDirSlot);

    m_pActionRename = new QAction(this);
    m_pActionRename->setText(QStringLiteral("重命名(&M)"));
    connect(m_pActionRename,
            &QAction::triggered,
            this,
            &Dialog::trigActionRenameSlot);

    m_pActionNewDir = new QAction(this);
    m_pActionNewDir->setText(QStringLiteral("新建文件夹(&N)"));
    QIcon newDirIcon(":/img/ftp/add.ico");
    //m_pActionNewDir->setIcon(newDirIcon);

    connect(m_pActionNewDir,
            &QAction::triggered,
            this,
            &Dialog::triActionNewDirSlot);
    connect(ui->treeWidgetFtp,
            &QTreeWidget::itemDoubleClicked,
            this,
            &Dialog::doubleclickItemSlot);
    connect(ui->treeWidgetFtp,
            &QTreeWidget::customContextMenuRequested,
            this,
            &Dialog::treeWdtContexMenuSlot);
    connect(ui->treeWidgetFtp,
            &QTreeWidget::itemClicked,
            this,
            &Dialog::clickItemSlot);
    connect(ui->treeWidgetFtp,
            &QTreeWidget::itemChanged,
            this,
            &Dialog::renameSlot);
    connect(ui->pushButtonHome,
            &QPushButton::clicked,
            this,
            &Dialog::homeBtnClickedSlot);
    connect(ui->pushButtonRefresh,
            &QPushButton::clicked,
            this,
            &Dialog::trigActionRefreshSlot);
    connect(ui->lineEditPath,
            &QLineEdit::returnPressed,
            this,
            &Dialog::lineDdieEnterSlot);
    connect(ui->lineEdit,
            &QLineEdit::textChanged,
            this,
            &Dialog::lineEditFindSlot);
    ui->treeWidgetFtp->setColumnWidth(0, 190);
    ui->treeWidgetFtp->setColumnWidth(1, 150);
    ui->treeWidgetFtp->setColumnWidth(2, 100);
    ui->treeWidgetFtp->header()->setSortIndicator(3,Qt::AscendingOrder);
    ui->treeWidgetFtp->setStyleSheet("QTreeWidget::item{height:20px}");

    QFont font;
    font.setPointSize(9);
    ui->treeWidgetFtp->setFont(font);
    //    font.setPointSize(12);
    //    ui->treeWidgetFtp->header()->setFont(font);
    QSize size;
    size.setHeight(30);
    ui->treeWidgetFtp->header()->setBaseSize(size);
    // font.setPointSize(12);
    // ui->treeWidgetFtp->header()->setFont(font);

    //    this->setFtpInfo("192.168.11.56","luoxiang","luoxiang",FTP_PROT);
    //  this->setFtpInfo("127.0.0.1","anonymous","bunfly@126.com", FTP_PROT);
    //    this->setFtpInfo(g_runtimeConfig.serverAddr().toString()
    //                     ,g_runtimeConfig.userName(),g_runtimeConfig.userPasswd(), FTP_PROT);
}

Dialog::~Dialog()
{
    m_pFtp->disconnect();
    m_pFtp->close();
    delete m_pFtp;

    delete m_pActionUploadDir;
    delete m_pActionDelete;
    delete m_pActionDownload;
    delete m_pActionNewDir;
    delete m_pActionOpen;
    delete m_pActionRefresh;
    delete m_pActionRename;
    delete m_pActionUpload;

    delete ui;
}

void Dialog::connectToFtp()
{
    m_pFtp->connectToHost(m_server, m_port);
    m_pFtp->login(m_username, m_password);
    m_pFtp->cd(stringToFtp(m_currentDir));
}

void Dialog::setFtpInfo(const QString &server, const QString &username,
                        const QString &password, quint16 port)
{
    m_server = server;
    m_username = username;
    m_password = password;
    m_port = port;
    this->connectToFtp();
}

QString Dialog::getFilePermissions(const QUrlInfo &urlInfo)
{
    QString permission;
    int p = urlInfo.permissions();
    if(urlInfo.isDir())
    {
        permission.append('d');
    }
    else
    {
        permission.append('-');
    }
    //owner permissions
    if(p & 0400) // owner r
    {
        permission.append('r');
    }
    else
    {
        permission.append('-');
    }

    if(p & 0200) // owner w
    {
        permission.append('w');
    }
    else
    {
        permission.append('-');
    }

    if(p & 0100) // owner x
    {
        permission.append('x');
    }
    else
    {
        permission.append('-');
    }
    //group permissions
    if(p & 0040) // group r
    {
        permission.append('r');
    }
    else
    {
        permission.append('-');
    }

    if(p & 0020) // group w
    {
        permission.append('w');
    }
    else
    {
        permission.append('-');
    }

    if(p & 0010) // group x
    {
        permission.append('x');
    }
    else
    {
        permission.append('-');
    }
    //other's permissions
    if(p & 0004) // other r
    {
        permission.append('r');
    }
    else
    {
        permission.append('-');
    }

    if(p & 0002) // other w
    {
        permission.append('w');
    }
    else
    {
        permission.append('-');
    }

    if(p & 0001) // other x
    {
        permission.append('x');
    }
    else
    {
        permission.append('-');
    }
    return permission;
}

QString Dialog::ftpToString(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return codec->toUnicode(input.toLatin1());
#else
    return input;
#endif
}

QString Dialog::stringToFtp(QString &input)
{
#if 1
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    return QString::fromLatin1(codec->fromUnicode(input));
#else
    return input;
#endif
}

void Dialog::doubleclickItemSlot(QTreeWidgetItem *treeWgtItem, int)
{
    if ( m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    }

    if (treeWgtItem->data(2,Qt::UserRole).toInt() != DIR && treeWgtItem->text(0) != "..") {
        this->trigActionDownloadSlot();
        return;
    }
    QString input = treeWgtItem->text(0);
    m_pFtp->cd(stringToFtp(input));
}

void Dialog::treeWdtContexMenuSlot(QPoint point)
{
    renameSlot(ui->treeWidgetFtp->currentItem(),0);
    QMenu menu;
    if ( ui->treeWidgetFtp->itemAt(point) == nullptr) {
        ui->treeWidgetFtp->clearSelection();
        ui->treeWidgetFtp->clearFocus();
        menu.addAction(m_pActionUpload);
        menu.addAction(m_pActionUploadDir);
        menu.addAction(m_pActionRefresh);
        menu.addAction(m_pActionNewDir);
    }
    else if ( ui->treeWidgetFtp->selectedItems().count() == 1) {
        if ( ui->treeWidgetFtp->currentItem()->data(2,Qt::UserRole) == DIR) {
            menu.addAction(m_pActionOpen);
            if ( ui->treeWidgetFtp->currentItem()->text(0) != "..") {
                menu.addAction(m_pActionDownload);
            }
        }
        else {
            menu.addAction(m_pActionDownload);
        }

        menu.addAction(m_pActionUpload);
        menu.addAction(m_pActionUploadDir);
        menu.addAction(m_pActionRefresh);
        menu.addAction(m_pActionNewDir);

        if ( ui->treeWidgetFtp->currentItem()->text(0) != "..") {
            menu.addAction(m_pActionRename);
            menu.addAction(m_pActionDelete);
        }
    }
    else {
        menu.addAction(m_pActionUpload);
        menu.addAction(m_pActionUploadDir);
        menu.addAction(m_pActionDownload);
        menu.addAction(m_pActionRefresh);
        menu.addAction(m_pActionNewDir);
        menu.addAction(m_pActionDelete);
    }
    menu.exec(this->cursor().pos());
}

void Dialog::ftpCommandStarted(int )
{
    int id = m_pFtp->currentCommand();
    switch (id)
    {
    case QFtp::ConnectToHost :
        break;
    case QFtp::Login :
        m_loginState = true;
        break;
    case QFtp::Get :
        break;
    case QFtp::Close :
        break;
    case QFtp::List :
        ui->treeWidgetFtp->clear();
        break;
    case QFtp::Mkdir :
        break;
    default:
        break;
    }
}

void Dialog::ftpCommandFinished(int , bool error)
{
    switch (m_pFtp->currentCommand()) {
    case QFtp::ConnectToHost:
        if (error) {
            m_pActionUpload->setEnabled(false);
            m_pActionNewDir->setEnabled(false);

            QMessageBox msgBox(this);
            msgBox.setWindowTitle(QStringLiteral("错误"));
            msgBox.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
            msgBox.setText(QStringLiteral("网络错误，连接服务器[%1]失败！")
                           .arg(m_server));
            msgBox.setStandardButtons(QMessageBox::Yes);
            msgBox.setButtonText(QMessageBox::Yes,QStringLiteral("确定"));
            msgBox.setDetailedText(m_pFtp->errorString());
            msgBox.exec();
            return;
        } else {
            m_pActionUpload->setEnabled(true);
            m_pActionNewDir->setEnabled(true);
        }
        break;
    case QFtp::Login:
        if(error)
        {
            m_pActionUpload->setEnabled(false);
            m_pActionNewDir->setEnabled(false);
            QMessageBox::critical(this,
                                  QStringLiteral("错误"),
                                  QStringLiteral("登录服务器失败，请确保文件服务已打开！"),
                                  QStringLiteral("确定"));
            return;
        }
        break;
    case QFtp::Get:
        if (error)
        {
            QMessageBox::warning(this,
                                 QStringLiteral("错误"),
                                 QStringLiteral("网络错误，下载[%1]失败！\n%2")
                                 .arg(m_pFile->fileName()).arg(m_pFtp->errorString()),
                                 QStringLiteral("确定"));
            return;
        }
        else
        {
            QMessageBox::warning(this,
                                 QStringLiteral("完成"),
                                 QStringLiteral("下载[%1]成功！").arg("file->fileName()"),
                                 QStringLiteral("确定"));
        }

        break;
    case QFtp::Put:
        if (error)
        {
            QMessageBox::warning(this,
                                 QStringLiteral("错误"),
                                 QStringLiteral("网络错误，上传[%1]失败！\n%2")
                                 .arg(m_pFile->fileName()).arg(m_pFtp->errorString()),
                                 QStringLiteral("确定"));
            return;
        }
        else
        {
            QMessageBox::warning(this,
                                 QStringLiteral("完成"),
                                 QStringLiteral("上传[%1]成功！")
                                 .arg(m_pFile->fileName()),
                                 QStringLiteral("确定"));
        }
        break;
    case QFtp::Rmdir:
        if (error)
        {
            QMessageBox::warning(this,
                                 QStringLiteral("错误"), QStringLiteral("%1")
                                 .arg(m_pFtp->errorString()),
                                 QStringLiteral("确定"));
        }
        break;
    case QFtp::Remove:
        if (error)
        {
            QMessageBox::warning(this,
                                 QStringLiteral("错误"),
                                 QStringLiteral("网络错误，删除[%1]失败！\n%2")
                                 .arg(m_pFile->fileName()).arg(m_pFtp->errorString()),
                                 QStringLiteral("确定"));
        }
        break;
    case QFtp::Rename:
        if (error)
        {
            QMessageBox::warning(this,
                                 QStringLiteral("错误"),
                                 QStringLiteral("网络错误，文件重命名失败！\n%1")
                                 .arg(m_pFtp->errorString()),
                                 QStringLiteral("确定"));
            m_pFtp->list();
        }
        break;
    case QFtp::List:
        if (error)
        {
            QMessageBox::warning(this,
                                 QStringLiteral("错误"),
                                 QStringLiteral("网络错误，获取文件列表失败！\n%1")
                                 .arg(m_pFtp->errorString()),
                                 QStringLiteral("确定"));
            return;
        }
        for ( int i = 0; i < ui->treeWidgetFtp->topLevelItemCount(); i++) {
            if ( ui->treeWidgetFtp->topLevelItem(i)->text(0) == "..") {
                return;
            }
        }

        if ( m_currentDir != m_rootDir ) {
            QTreeWidgetItem *item = new QTreeWidgetItem;
            QIcon icon(":/img/ftp/dir.png");
            item->setIcon(0, icon);
            item->setText(0,"..");
            //  item->setData(0,);
            ui->treeWidgetFtp->addTopLevelItem(item);
        }

        break;
    case QFtp::Cd :
        if (error)
        {
            QMessageBox::warning(this,
                                 QStringLiteral("错误"),
                                 QStringLiteral("网络错误，切换目录失败！\n%1")
                                 .arg(m_pFtp->errorString()),
                                 QStringLiteral("确定"));
            ui->lineEditPath->setText(m_currentDir);
            return;
        }
        m_pFtp->list();

        if ( m_loginState) {
            m_loginState = false;
            return;
        }
        if ( ui->treeWidgetFtp->currentItem() == nullptr) {
            if ( ui->lineEditPath->text().isEmpty()) {
                m_currentDir = m_rootDir;
                ui->lineEditPath->setText(m_currentDir);
            }
            else {
                m_currentDir = ui->lineEditPath->text().trimmed();
            }
            return;
        }
        if ( ui->treeWidgetFtp->currentItem()->text(0) == "..") {
            m_currentDir = m_currentDir.left(m_currentDir.lastIndexOf("/"));
            if ( m_currentDir.isEmpty()) {
                m_currentDir.append("/");
            }
        }
        else if ( m_currentDir != m_rootDir){
            m_currentDir.append(QStringLiteral("/%0")
                                .arg(ui->treeWidgetFtp->currentItem()->text(0)));
        }
        else {
            m_currentDir.append(QStringLiteral("%0")
                                .arg(ui->treeWidgetFtp->currentItem()->text(0)));
        }
        ui->lineEditPath->setText(m_currentDir);
        break;
    case QFtp::Mkdir :
        if ( error) {
            m_pFtp->list();
        }
        break;
    default:
        break;
    }
}

void Dialog::stateChangedSlot(int state)
{
    switch (state) {
    case QFtp::ConnectToHost:
        emit sendState(QStringLiteral("连接到服务器..."));
        break;
    case QFtp::Login :
        emit sendState(QStringLiteral("已登录..."));
        m_pActionDelete->setEnabled(true);
        m_pActionDownload->setEnabled(true);
        m_pActionNewDir->setEnabled(true);
        m_pActionOpen->setEnabled(true);
        m_pActionRename->setEnabled(true);
        m_pActionUpload->setEnabled(true);
        break;
    case QFtp::Close :
        emit sendState(QStringLiteral("断开连接..."));
        break;
    case QFtp::Get :
        emit sendState(QStringLiteral("下载中..."));
        break;
    case QFtp::Put :
        emit sendState(QStringLiteral("上传中..."));
        break;
    case QFtp::Unconnected :
        m_pActionDelete->setEnabled(false);
        m_pActionDownload->setEnabled(false);
        m_pActionNewDir->setEnabled(false);
        m_pActionOpen->setEnabled(false);
        m_pActionRename->setEnabled(false);
        m_pActionUpload->setEnabled(false);
    default:
        break;
    }
}

void Dialog::ListInfoSlot(const QUrlInfo &info)
{
    QString name = info.name();

    if ( ftpToString(name) == "." || ftpToString(name) == "..") {
        return;
    }

    QTreeWidgetItem *item = new QTreeWidgetItem;

    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(0, ftpToString(name));
    item->setText(1, info.lastModified().toString("yyyy-MM-dd hh:mm"));
    item->setText(3, getFilePermissions(info));

    QFileInfo fileInfo(item->text(0));
    if ( info.isDir()) {
        item->setText(2, QStringLiteral("文件夹"));
        item->setData(2, Qt::UserRole,DIR);
        item->setIcon(0, QIcon(":/img/ftp/dir.png"));
    }
    else {
        //        item->setText(2, fileInfo.suffix() + QStringLiteral("文件"));
        item->setText(2, fileInfo.suffix());
        item->setData(2, Qt::UserRole,OTHER);
        item->setIcon(0, QIcon(":/img/ftp/file.png"));

        int integer = info.size() / 1024;
        int remainder = info.size() % 1024;
        QString number = QString::number(integer) + '.' + QString::number(remainder);
        item->setText(4, QString::number(number.toFloat() / 1024, 'f', 2) + "MB");
    }


    ui->treeWidgetFtp->addTopLevelItem(item);
}

void Dialog::dataTransferProgressSlot(qint64 sum, qint64 size, QProgressBar *bar, QString taskName)
{
    if( bar == nullptr) {
        return;
    }
    ui->scrollArea->setVisible(true);
    bar->setToolTip(taskName);
    bar->setMaximum(size / 1024);
    bar->setValue(sum  / 1024);
}

void Dialog::threadDoneSlot(bool flag,QProgressBar *bar)
{
    if ( bar == nullptr) {
        this->trigActionRefreshSlot();
        // this->resize(687,363);
        return;
    }
    m_threadMutex.lock();

    m_threadNumber--;
    m_threadMutex.unlock();
    ui->layout1->removeWidget(bar);
    delete bar;
    bar = nullptr;

    if ( !flag ) {
        FTPClient *thread = dynamic_cast<FTPClient *>(QObject::sender());
        QMessageBox::critical(this,
                              QStringLiteral("错误"),
                              thread->getLastErrMsg(),
                              QMessageBox::Yes,
                              QMessageBox::Yes);

    }
    if ( 0 == m_threadNumber) {
        this->trigActionRefreshSlot();
        // this->resize(687,363);
        ui->scrollArea->setVisible(false);
    }
    else {
        return;
    }
}

void Dialog::homeBtnClickedSlot()
{
    ui->treeWidgetFtp->setCurrentItem(nullptr);
    m_currentDir = m_rootDir;
    ui->lineEditPath->setText(m_rootDir);
    m_pFtp->cd(m_rootDir);
}

void Dialog::lineDdieEnterSlot()
{
    if ( ui->lineEditPath->text().trimmed() == m_currentDir) return;
    QString input = ui->lineEditPath->text().trimmed();
    m_pFtp->cd(stringToFtp(input));
}

void Dialog::lineEditFindSlot(QString text)
{
    if ( text.trimmed().isEmpty()) {
        ui->treeWidgetFtp->clearSelection();
        ui->treeWidgetFtp->setCurrentItem(nullptr);
        return;
    }
    for ( int i = 0; i < ui->treeWidgetFtp->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui->treeWidgetFtp->topLevelItem(i);
        int index = item->text(0).indexOf(text.trimmed());
        if ( index == 0) {
            item->setSelected(true);
            ui->treeWidgetFtp->setCurrentItem(item);
            break;
        }
    }
}

void Dialog::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void Dialog::dropEvent(QDropEvent *event)
{
    QList<QUrl> urlList = event->mimeData()->urls();
    if ( urlList.isEmpty()) {
        event->ignore();
        return;
    }
    foreach (QUrl url, urlList) {
        QString filePath = url.toLocalFile();
        if ( filePath.isEmpty()) continue;
        QFileInfo info(filePath);
        if ( info.isDir()) {
            uploadDir(filePath);
        }
        else {
            QStringList list = QStringList(filePath);
            uploadFile(list);
        }
    }
    event->acceptProposedAction();
}

void Dialog::trigActionDeleteSlot()
{
    if ( m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    }

    if (m_pFtp->state() != QFtp::Login) {//可能是LoggedIn
        QMessageBox::critical(this,
                              QStringLiteral("错误"),
                              QStringLiteral("与服务器连接异常!"),
                              QStringLiteral("确定"));
        return;
    }

    QList<QTreeWidgetItem*> itemList = ui->treeWidgetFtp->selectedItems();
    if ( itemList.isEmpty()) return;

    int ret = QMessageBox::warning(this,
                                   QStringLiteral("警告"),
                                   QStringLiteral("删除后无法恢复，是否继续？"),
                                   QStringLiteral("确定"),
                                   QStringLiteral("取消"),nullptr,1);
    if ( ret) return;

    QStringList dirList;
    QStringList fileList;
    dirList.clear();
    fileList.clear();

    foreach (QTreeWidgetItem *item,itemList) {
        if ( item == nullptr) continue;
        if ( item->data(2,Qt::UserRole) == DIR) {
            dirList.append(item->text(0));
        }
        else if ( item->data(2,Qt::UserRole) == OTHER){
            fileList.append(item->text(0));
            ui->treeWidgetFtp->takeTopLevelItem(ui->treeWidgetFtp->indexOfTopLevelItem(item));
        }
        else {
            continue;
        }
    }

    foreach (QString dirPath, dirList) {
        if ( dirPath.isEmpty()) continue;
        FTPClient *thread = new FTPClient(this);
        connect(thread,
                &FTPClient::ftpDone,
                thread,
                &FTPClient::deleteLater);
        connect(thread,
                &FTPClient::ftpDone,
                this,
                &Dialog::threadDoneSlot);
        thread->setServer(m_server,m_username,m_password);
        thread->rmDir(stringToFtp(m_currentDir),
                      stringToFtp(dirPath));
    }
    foreach (QString filePath, fileList) {
        if ( filePath.isEmpty()) continue;
        m_pFtp->remove(stringToFtp(filePath));
    }
}

void Dialog::trigActionDownloadSlot()
{
    if ( m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    }
    if (m_pFtp->state() != QFtp::Login) {//可能是LoggedIn
        QMessageBox::critical(this,
                              QStringLiteral("错误"),
                              QStringLiteral("与服务器连接异常!"),
                              QStringLiteral("确定"));
        return;
    }

    QString saveDirPath = QFileDialog::getExistingDirectory(this,"Open Dir","");
    if ( saveDirPath.isEmpty()) return;

    QList<QTreeWidgetItem*> itemList = ui->treeWidgetFtp->selectedItems();
    QStringList dirList;
    QStringList fileList;
    dirList.clear();
    fileList.clear();
    foreach (QTreeWidgetItem *item, itemList) {
        if ( item == nullptr) continue;
        if ( item->data(2,Qt::UserRole) == DIR) {
            if ( item->text(0) == "..") {
                continue;
            }
            dirList.append(item->text(0));
        }
        else if ( item->data(2,Qt::UserRole) == OTHER){
            fileList.append(item->text(0));
        }
        else {
            continue;
        }
    }

    foreach (QString dirPath, dirList) {
        if ( dirPath.isEmpty()) continue;
        this->downloadDir(saveDirPath,dirPath);
    }

    foreach (QString filePath, fileList) {
        if ( filePath.isEmpty()) continue;
        this->downloadFile(saveDirPath,filePath);
    }
}

void Dialog::downloadFile(QString &savePath,QString &fileName)
{
    QFile file;
    file.setFileName(QStringLiteral("%0/%1")
                     .arg(savePath)
                     .arg(fileName));
    if ( file.exists()) {
        int ret = QMessageBox::warning(this,
                                       QStringLiteral("警告"),
                                       QStringLiteral("检测到目标路径已存在该文件!"),
                                       QStringLiteral("替换"),
                                       QStringLiteral("都保存"),
                                       QStringLiteral("取消"), 2);
        int k = 1;
        QString baseName = QFileInfo(file.fileName()).baseName();
        QString suffix = QFileInfo(file.fileName()).suffix();
        switch (ret) {
        case 0:
            break;
        case 2:
            return;
            break;
        case 1:
            while (1) {
                file.setFileName(QStringLiteral("%0/%1(%2)")
                                 .arg(savePath).arg(baseName)
                                 .arg(QString::number(k)));
                if ( !suffix.isEmpty()) {
                    file.setFileName(file.fileName() + "." + suffix);
                }

                if ( !file.exists()) break;
                k++;
            }
            break;
        default:
            break;
        }
    }

    m_threadMutex.lock();
    FTPClient *thread = new FTPClient(this);
    QProgressBar *m_progress = new QProgressBar(this);
    m_progress->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_progress->setToolTip(fileName);
    m_progress->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->layout1->addWidget(m_progress);
    connect(thread,
            &FTPClient::ftpDone,
            thread,
            &FTPClient::deleteLater);
    connect(thread,
            &FTPClient::dataTransferProgressSignal,
            this,
            &Dialog::dataTransferProgressSlot);
    connect(thread,
            &FTPClient::ftpDone,
            this,
            &Dialog::threadDoneSlot);
    thread->setServer(m_server,m_username,m_password);
    thread->recvFile(file.fileName(),stringToFtp(m_currentDir),
                     stringToFtp(fileName),m_progress);
    m_threadNumber++;
    m_threadMutex.unlock();
}

void Dialog::downloadDir(QString &savePath, QString &dirName)
{
    QDir dir(QStringLiteral("%0/%1")
             .arg(savePath)
             .arg(dirName));
    if ( dir.exists()) {
        int ret = QMessageBox::warning(this,
                                       QStringLiteral("警告"),
                                       QStringLiteral("检测到目标路径已存在该文件!"),
                                       QStringLiteral("合并且覆盖"),
                                       QStringLiteral("都保存"),
                                       QStringLiteral("取消"), 2);
        int k = 1;
        switch (ret) {
        case 0:
            break;
        case 2:
            return;
            break;
        case 1:
            while (1) {
                dir.setPath(QStringLiteral("%0/%1(%2)")
                            .arg(savePath).arg(dirName)
                            .arg(QString::number(k)));
                if ( !dir.exists()) break;
                k++;
            }
            break;
        default:
            break;
        }
    }


    m_threadMutex.lock();
    FTPClient *thread = new FTPClient(this);
    QProgressBar *m_progress = new QProgressBar(this);
    m_progress->setToolTip(dirName);
    m_progress->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->layout1->addWidget(m_progress);
    connect(thread,
            &FTPClient::ftpDone,
            thread,
            &FTPClient::deleteLater);
    connect(thread,
            &FTPClient::dataTransferProgressSignal,
            this,
            &Dialog::dataTransferProgressSlot);
    connect(thread,
            &FTPClient::ftpDone,this,
            &Dialog::threadDoneSlot);
    thread->setServer(m_server,m_username,m_password);
    thread->recvDir(dir.path(),stringToFtp(m_currentDir),
                    stringToFtp(dirName),m_progress);
    m_threadNumber++;
    m_threadMutex.unlock();
}

void Dialog::trigActionRefreshSlot()
{
    if (m_pFtp->state() == QFtp::Connecting) {
        return;
    }
    if (m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    } else {
        m_pFtp->list();
    }
}

void Dialog::trigActionOpenSlot()
{
    if ( m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    }

    this->doubleclickItemSlot(ui->treeWidgetFtp->currentItem(),
                              ui->treeWidgetFtp->currentColumn());
}

void Dialog::trigActionUploadSlot()
{
    QStringList filePathList = QFileDialog::getOpenFileNames(this,
                                                             QStringLiteral("打开文件"),
                                                             QDir::currentPath());
    if ( filePathList.isEmpty()) return;
    uploadFile(filePathList);

}

void Dialog::trigActionUploadDirSlot()
{
    QString dirPath = QFileDialog::getExistingDirectory(this,
                                                        QStringLiteral("打开文件"),
                                                        QDir::currentPath());
    if ( dirPath.isEmpty()) return;
    uploadDir(dirPath);

}

void Dialog::uploadFile(QStringList &fileList)
{
    if ( m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    }
    if (m_pFtp->state() != QFtp::Login) {
        QMessageBox::critical(this,
                              QStringLiteral("错误"),
                              QStringLiteral("连接异常!"),
                              QStringLiteral("确定"));
        return;
    }

    foreach (QString filePath, fileList) {
        if ( filePath.isEmpty()) continue;
        QFileInfo fileInfo(filePath);
        bool ok = false;
        for ( int i = 0; i < ui->treeWidgetFtp->topLevelItemCount(); i++) {
            QTreeWidgetItem *item = ui->treeWidgetFtp->topLevelItem(i);
            if ( item->text(0) == "..") continue;
            if ( item->data(2,Qt::UserRole) == OTHER &&
                 item->text(0) == fileInfo.fileName()) {
                int ret =QMessageBox::warning(this,
                                              QStringLiteral("警告"),
                                              QString(QStringLiteral("文件[%0]已存在！\n（建议：重命名文件后上传）"))
                                              .arg(fileInfo.fileName()),
                                              QStringLiteral("取消"),
                                              QStringLiteral("覆盖")
                                              );

                if ( !ret) {
                    ok = true;
                    break;
                }
                else {
                    ok = false;
                    break;
                }
            }
        }

        if ( ok) {
            continue;
        }

        m_threadMutex.lock();
        FTPClient *thread = new FTPClient(this);
        QProgressBar *m_progress = new QProgressBar(this);
        m_progress->setToolTip(fileInfo.fileName());
        m_progress->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->layout1->addWidget(m_progress);
        connect(thread,
                &FTPClient::ftpDone,
                thread,
                &FTPClient::deleteLater);
        connect(thread,
                &FTPClient::dataTransferProgressSignal,
                this,
                &Dialog::dataTransferProgressSlot);
        connect(thread,
                &FTPClient::ftpDone,
                this,
                &Dialog::threadDoneSlot);
        thread->setServer(m_server,m_username,m_password);
        QString fileName = fileInfo.fileName();
        thread->sendFile(filePath,stringToFtp(m_currentDir),
                         stringToFtp(fileName),m_progress);
        m_threadNumber++;
        m_threadMutex.unlock();
    }
}

void Dialog::uploadDir(QString &dirPath)
{
    if ( m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    }
    if (m_pFtp->state() != QFtp::Login) {
        QMessageBox::critical(this,
                              QStringLiteral("错误"),
                              QStringLiteral("连接异常!"),
                              QStringLiteral("确定"));
        return;
    }

    QDir dir(dirPath);
    for ( int i = 0; i < ui->treeWidgetFtp->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui->treeWidgetFtp->topLevelItem(i);
        if ( item->text(0) == "..") continue;
        if ( item->data(2,Qt::UserRole) == DIR &&
             item->text(0) == dir.dirName()) {

            QMessageBox::warning(this,
                                 QStringLiteral("警告"),
                                 QString(QStringLiteral("文件夹[%0]已存在！\n（建议：重命名文件夹后上传）"))
                                 .arg(dir.dirName()),
                                 QStringLiteral("确定"));
            return;
        }
    }

    m_threadMutex.lock();
    FTPClient *thread = new FTPClient(this);
    QProgressBar *m_progress = new QProgressBar(this);
    m_progress->setToolTip(dir.dirName());
    m_progress->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->layout1->addWidget(m_progress);
    connect(thread,
            &FTPClient::ftpDone,
            thread,
            &FTPClient::deleteLater);
    connect(thread,
            &FTPClient::dataTransferProgressSignal,
            this,
            &Dialog::dataTransferProgressSlot);
    connect(thread,
            &FTPClient::ftpDone,
            this,
            &Dialog::threadDoneSlot);
    thread->setServer(m_server,m_username,m_password);
    thread->sendDir(dirPath.left(dirPath.lastIndexOf("/") + 1),m_currentDir,dirPath,m_progress);
    m_threadNumber++;
    m_threadMutex.unlock();
}

void Dialog::trigActionRenameSlot()
{
    m_oldName = ui->treeWidgetFtp->currentItem()->text(0);
    m_oldItem = ui->treeWidgetFtp->currentItem();
    ui->treeWidgetFtp->editItem(ui->treeWidgetFtp->currentItem(),0);
}

void Dialog::renameSlot(QTreeWidgetItem *item, int)
{
    if ( m_oldItem == nullptr|| item != m_oldItem) {
        m_oldItem = nullptr;
        return;
    }
    if ( m_oldName == item->text(0)) {
        m_oldItem = nullptr;
        return;
    }
    if ( m_pFtp->state() == QFtp::Unconnected) {
        this->connectToFtp();
    }

    QRegExp rx("^[^|/\"\\\\<>*?:]*$");//字符不能包含\/:*?"<>| 同时不能以空格开头
    if ( !rx.exactMatch(m_oldItem->text(0)) || m_oldItem->text(0).startsWith(" ")) {
        QMessageBox::warning(this,QStringLiteral("错误"),
                             QStringLiteral("文件名不能以空格开始且不能包含下列字符：\n \\ / : * ? \" < > |"),
                             QStringLiteral("确定"));
        m_oldItem->setText(0,m_oldName);
        m_oldItem = nullptr;
        m_oldName.clear();
        return;
    }
    QString oldNmae = stringToFtp(m_oldName);
    QString newName = m_oldItem->text(0);
    newName = stringToFtp(newName);
    m_pFtp->rename(oldNmae,
                   newName);
    m_oldName.clear();
    m_oldItem = nullptr;
}

void Dialog::clickItemSlot(QTreeWidgetItem *item, int i)
{
    static QTreeWidgetItem *lastItem = nullptr;
    if ( 0 == i) {
        if ( item == lastItem && lastItem != nullptr&& item->text(0) != "..") {
            lastItem = nullptr;
            m_oldName = ui->treeWidgetFtp->currentItem()->text(0);
            m_oldItem = ui->treeWidgetFtp->currentItem();
            ui->treeWidgetFtp->editItem(ui->treeWidgetFtp->currentItem(),0);
        }
        else {
            lastItem = item;
        }
    }
    else {
        lastItem = item;
    }
}

void Dialog::triActionNewDirSlot()
{
    QString dirName = QStringLiteral("新建文件夹");
    bool ok = true;
    int j = 1;
    while ( ok) {
        for ( int i = 0; i < ui->treeWidgetFtp->topLevelItemCount(); i++) {
            if ( ui->treeWidgetFtp->topLevelItem(i)->data(2,Qt::UserRole).toInt() != DIR) continue;
            if ( ui->treeWidgetFtp->topLevelItem(i)->text(0) == dirName) {
                ok = false;
                break;
            }
        }
        if ( ok) break;

        dirName = QStringLiteral("新建文件夹(%0)").arg(QString::number(j));
        j++;
        ok = true;
    }

    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    QPixmap pixmap(":/img/ftp/dir.png");
    item->setIcon(0, pixmap);
    item->setText(0,dirName);
    item->setText(2,QStringLiteral("文件夹"));
    item->setData(2,Qt::UserRole,DIR);
    item->setText(3,"drw-rw-rw-");
    item->setText(1,QDateTime::currentDateTime().toString(QString("yyyy-MM-dd hh:mm")));
    ui->treeWidgetFtp->addTopLevelItem(item);
    ui->treeWidgetFtp->editItem(item);
    QString dir = item->text(0);
    dir =  stringToFtp(dir);
    m_pFtp->mkdir(dir);

    m_oldName = dirName;
    m_oldItem = item;
}
