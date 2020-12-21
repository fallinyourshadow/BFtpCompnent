#ifndef FTPCLIENTAPI_H
#define FTPCLIENTAPI_H

#include <QObject>

class FtpClientAPI : public QObject
{
    Q_OBJECT
public:
    explicit FtpClientAPI(QObject *parent = nullptr);

signals:

public slots:
};

#endif // FTPCLIENTAPI_H
