#include "TestUi.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestUi w;
    w.show();
    return a.exec();
}
