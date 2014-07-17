#include "kaqazsync.h"
#include "sialandeveloper_macros.h"
#include "kaqazmacros.h"
#include "kaqazdropbox.h"
#include "qdropboxfile.h"
#include "database.h"
#include "kaqaz.h"

#include <QThread>
#include <QPointer>

class KaqazSyncPrivate
{
public:
    KaqazDropBox *kdbox;
    QPointer<QThread> thread;
    Database *db;

    QString password;
};

KaqazSync::KaqazSync(Database *db, QObject *parent) :
    QObject(parent)
{
    p = new KaqazSyncPrivate;
    p->db = db;
    p->password = p->db->syncPassword();

    connect( p->db, SIGNAL(paperChanged(int))  , SLOT(refresh()) );
    connect( p->db, SIGNAL(revisionsChanged()) , SLOT(refresh()) );
    connect( p->db, SIGNAL(filesListChanged()) , SLOT(refresh()) );

    reload();
}

bool KaqazSync::tokenAvailable() const
{
    return p->kdbox->tokenAvailable();
}

void KaqazSync::setPassword(const QString &pass)
{
    QString hash = Kaqaz::passToMd5(pass);
    p->db->setSyncPassword( hash );
    p->password = hash;
}

QString KaqazSync::password() const
{
    return p->db->syncPassword();
}

void KaqazSync::password_changed(const QString &password)
{
    p->db->setSyncPassword( password );
    emit passwordChanged();
}

void KaqazSync::connectedChanged()
{
    if( p->kdbox->connected() )
    {
        connect( p->db, SIGNAL(paperChanged(int))  , p->kdbox, SLOT(refresh()), Qt::QueuedConnection );
        connect( p->db, SIGNAL(groupsListChanged()), p->kdbox, SLOT(refresh()), Qt::QueuedConnection );

        start();
    }
    else
    {
        disconnect( p->db, SIGNAL(paperChanged(int))  , p->kdbox, SLOT(refresh()) );
        disconnect( p->db, SIGNAL(groupsListChanged()), p->kdbox, SLOT(refresh()) );
    }
}

void KaqazSync::refresh()
{
    p->kdbox->setLocalSyncHash( p->db->revisions() );
    QMetaObject::invokeMethod( p->kdbox, "refresh", Qt::QueuedConnection );
}

void KaqazSync::refreshForce()
{
    p->kdbox->setLocalSyncHash( p->db->revisions() );
    QMetaObject::invokeMethod( p->kdbox, "refreshForce", Qt::QueuedConnection );
}

void KaqazSync::start()
{
    QMetaObject::invokeMethod( p->kdbox, "connectDropbox", Qt::QueuedConnection, Q_ARG(QString,p->password) );
}

void KaqazSync::stop()
{
    QSettings settings(CONFIG_PATH,QSettings::IniFormat);
    settings.remove(TOKEN_KEY);
    settings.remove(TOKEN_SECRET);
    reload();
}

void KaqazSync::reload()
{
    if( p->thread )
    {
        p->thread->quit();
        p->thread->wait();
        p->thread->deleteLater();
    }

    p->thread = new QThread(this);

    p->kdbox = new KaqazDropBox();
    p->kdbox->setLocalSyncHash(p->db->revisions());
    p->kdbox->moveToThread(p->thread);

    connect( p->kdbox, SIGNAL(authorizeRequest())      , SIGNAL(authorizeRequest())     , Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(passwordChanged(QString)), SLOT(password_changed(QString)), Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(paperDeleted(QString))   , SLOT(paperDeleted(QString))    , Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(connectedChanged())      , SLOT(connectedChanged())       , Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(syncStarted())           , SIGNAL(syncStarted())          , Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(syncProgress(qreal))     , SIGNAL(syncProgress(qreal))    , Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(syncFinished())          , SIGNAL(syncFinished())         , Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(tokenAvailableChanged()) , SIGNAL(tokenAvailableChanged()), Qt::QueuedConnection );
    connect( p->kdbox, SIGNAL(refreshRequest())        , SLOT(refreshForce())           , Qt::QueuedConnection );

    connect( p->thread, SIGNAL(finished()), p->kdbox, SLOT(deleteLater()) );

    p->thread->start();
    QMetaObject::invokeMethod( p->kdbox, "initialize", Qt::QueuedConnection );

    emit tokenAvailableChanged();
}

void KaqazSync::authorizeDone()
{
    QMetaObject::invokeMethod( p->kdbox, "authorizeDone", Qt::QueuedConnection );
}

void KaqazSync::paperDeleted(const QString &id)
{
    int paperId = p->db->paperUuidId(id);
    if( paperId == -1 )
        return;

    p->db->deletePaper( paperId );
}

KaqazSync::~KaqazSync()
{
    delete p;
}