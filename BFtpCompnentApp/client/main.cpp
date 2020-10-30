#include <QApplication>
#include "dialog.h"
#include "../runtimeconfig.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog * d = new Dialog;
    d->show();
    d->setFtpInfo(SERVER_IP,USER_NAME,PASS_WORD,FTP_PROT);
    return a.exec();
}
