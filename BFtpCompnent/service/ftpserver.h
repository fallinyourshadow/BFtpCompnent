#ifndef FTPSERVER_H
#define FTPSERVER_H
#include <QMap>
#include <QString>
#include <QMutex>

class CFtpServer;
class FtpServer
{
public:
    FtpServer();
    ~FtpServer();
    bool init(unsigned short port);
    void close();
    bool addUser(const QString &name, const QString &passwd, const QString &home);
    bool editUser(const QString &name, const QString &passwd, const QString &home);
    bool deleteUser(const QString &name);
    bool userExists(const QString &name);
private:
    CFtpServer *m_pFtpServer;
    QMap<QString , void *> m_userMap;
    QMutex m_lock;
};

#endif // FTPSERVER_H
