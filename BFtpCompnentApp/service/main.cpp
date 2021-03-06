#include <QApplication>
#include "../runtimeconfig.h"
#include "ftpserver.h"
#include <QWidget>
#include <QDebug>
#include <WinSock2.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FtpServer *s  = new FtpServer;
    QWidget w;
    if(s->init(FTP_PROT))
    {
        w.show();
    }

    if(!s->userExists(USER_NAME))
    {
       // qDebug() << WSAGetLastError();
        s->addUser(USER_NAME,PASS_WORD,ROOT_DIR);
    }
    else
    {
        a.exit(0);
    }
    return a.exec();
}
