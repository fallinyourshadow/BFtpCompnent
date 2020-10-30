#include <QDir>
#include <QDebug>

#include "ftpserver.h"
#include "CFtpServer.h"

#define FTPSERVER_DEBUG

FtpServer::FtpServer()
{
    m_pFtpServer = NULL;
}

FtpServer::~FtpServer()
{
    close();
}

bool FtpServer::init(unsigned short port)
{
    if(m_pFtpServer)
    {
        close();
    }
    m_pFtpServer = new CFtpServer;
    //    m_pFtpServer->SetCheckPassDelay(3);
    m_pFtpServer->SetMaxPasswordTries( 3 );
    m_pFtpServer->SetNoLoginTimeout( 45 ); // seconds
    m_pFtpServer->SetNoTransferTimeout( 90 ); // seconds
    m_pFtpServer->SetDataPortRange( 100, 900 ); // data TCP-Port range = [100-999]

    int iPort = port;

    unsigned long ulLocalInterface = INADDR_ANY;

    if(!m_pFtpServer->StartListening(ulLocalInterface, (unsigned short)iPort))
    {
        return false;
    }
    if(!m_pFtpServer->StartAccepting())
    {
        return false;
    }
    return true;
}
void FtpServer::close()
{
    if(m_pFtpServer)
    {
        if(m_pFtpServer->IsListening())
        {
            m_pFtpServer->StopListening();
        }

        delete m_pFtpServer;
    }
    m_pFtpServer = NULL;
}
bool FtpServer::addUser(const QString &name, const QString &passwd, const QString &home)
{

    if(home.isEmpty() || name.isEmpty())
    {
        return false;
    }
    if(userExists(name))
    {
        qDebug() << "Ftp Server add user failed: user exists.";
        return false;
    }
    QDir dir;
    dir.setPath(home);
    if(!dir.exists() && !dir.mkpath(home))
    {
        qDebug() << "Ftp Server mkdir failed: " << home;
        return false;
    }
    m_lock.lock();
    unsigned char ucUserPriv = CFtpServer::LIST;

    ucUserPriv |= CFtpServer::READFILE | CFtpServer::WRITEFILE | CFtpServer::DELETEFILE;
    ucUserPriv |= CFtpServer::CREATEDIR | CFtpServer::DELETEDIR;
    CFtpServer::CUserEntry *pUser = m_pFtpServer->AddUser(name.toStdString().c_str(),
                                                          passwd.toStdString().c_str(),
                                                          home.toStdString().c_str());
    if(pUser)
    {
        //qDebug() << "Ftp Server add user success: " << name << passwd << home;
        pUser->SetMaxNumberOfClient(0);
        pUser->SetPrivileges(ucUserPriv);
        m_userMap.insert(name, static_cast<void *>(pUser));
        m_lock.unlock();
        return true;
    }else
    {
        //qDebug() << "Ftp Server add user failed: " << name << passwd;
        m_lock.unlock();
        return false;
    }
}

bool FtpServer::editUser(const QString &name, const QString &passwd, const QString &home)
{
    if(userExists(name))
    {
        deleteUser(name);
        return addUser(name, passwd, home);
    }
    return false;

}

bool FtpServer::deleteUser(const QString &name)
{
    if(!userExists(name))
    {
        return true;
    }
    CFtpServer::CUserEntry *pUser = NULL;
    m_lock.lock();
    pUser = static_cast<CFtpServer::CUserEntry *>(m_userMap[name]);
    if(pUser)
    {
        if(m_pFtpServer->DeleteUser(pUser))
        {
            m_userMap.remove(name);
            m_lock.unlock();

            //qDebug() << "delete user " << name;
            return true;
        }
    }
    m_lock.unlock();
    return false;
}

bool FtpServer::userExists(const QString &name)
{
    bool b = false;
    m_lock.lock();
    if(m_userMap[name])
    {
        b = true;
    }
    m_lock.unlock();
    return b;
}



