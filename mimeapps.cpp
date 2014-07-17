#define GLOBAL_APPS_PATH QString("/usr/share/applications")
#define LOCAL_APPS_PATH QString(QDir::homePath() + "/.local/share/applications")

#include "mimeapps.h"

#include <QDir>
#include <QHash>
#include <QSettings>
#include <QProcess>
#include <QMimeDatabase>
#include <QMimeType>
#include <QFile>
#include <QRegExp>
#include <QDebug>

class MimeAppsItem
{
public:
    QString name;
    QString icon;
    QString genericName;
    QString comment;
    QString path;
    QString command;
    QStringList mimes;
};

QMultiHash<QString,QString> mime_apps_apps;
QHash<QString,MimeAppsItem> mime_apps_items;

QStringList filesOf( const QString & path )
{
    QStringList res;

    const QStringList & dirs = QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach( const QString & d, dirs )
        res << filesOf( path + "/" + d );

    const QStringList & files = QDir(path).entryList(QStringList()<<"*.desktop",QDir::Files);
    foreach( const QString & f, files )
        res << path + "/" + f;

    return res;
}

QHash<QString,QString> readConfFile( const QString & file )
{
    QHash<QString,QString> res;

    QFile f(file);
    if( !f.open(QFile::ReadOnly) )
        return res;

    const QString & data = f.readAll();
    f.close();

    QRegExp mexr("\\[(.*)\\]\\s((?:\\s|.)*)\\s(?:\\[|$)");
    mexr.setMinimal(true);

    int mpos = 0;
    while ((mpos = mexr.indexIn(data, mpos)) != -1)
    {
        const QString & section    = mexr.cap(1);
        const QString & properties = mexr.cap(2);

        QRegExp pexr("(?:\\r|\\n|^)(.*)\\=(.*)(?:\\r|\\n)");
        pexr.setMinimal(true);

        int ppos = 0;
        while ((ppos = pexr.indexIn(properties, ppos)) != -1)
        {
            const QString & key   = pexr.cap(1);
            const QString & value = pexr.cap(2);

            res[ section + "/" + key ] = value;

            ppos += pexr.matchedLength()-1;
        }

        mpos += mexr.matchedLength()-1;
    }

    return res;
}

void init_mimeApps()
{
    if( !mime_apps_items.isEmpty() )
        return;

    QStringList desktops;
    desktops << filesOf(GLOBAL_APPS_PATH);
    desktops << filesOf(LOCAL_APPS_PATH);

    foreach( const QString & d, desktops )
    {
        const QHash<QString,QString> & conf = readConfFile(d);

        MimeAppsItem item;
        item.name        = conf.value("Desktop Entry/Name");
        item.icon        = conf.value("Desktop Entry/Icon");
        item.genericName = conf.value("Desktop Entry/GenericName");
        item.comment     = conf.value("Desktop Entry/Comment");
        item.path        = conf.value("Desktop Entry/Path");
        item.command     = conf.value("Desktop Entry/Exec");
        item.mimes       = conf.value("Desktop Entry/MimeType").split(QRegExp("(\\;|\\:)"),QString::SkipEmptyParts);

        foreach( const QString & m, item.mimes )
        {
            mime_apps_apps.insertMulti( m.toLower(), d );
            mime_apps_items.insert( d, item );
        }
    }
}

class MimeAppsPrivate
{
public:
    QMimeDatabase mdb;
};

MimeApps::MimeApps(QObject *parent) :
    QObject(parent)
{
    init_mimeApps();

    p = new MimeAppsPrivate;
}

QStringList MimeApps::appsOfMime(const QString &mime)
{
    return mime_apps_apps.values(mime.toLower());
}

QStringList MimeApps::appsOfFile(const QString &file)
{
    const QMimeType & type = p->mdb.mimeTypeForFile(file);
    return appsOfMime(type.name());
}

QString MimeApps::appName(const QString &app) const
{
    return mime_apps_items.value(app).name;
}

QString MimeApps::appIcon(const QString &app) const
{
    return mime_apps_items.value(app).icon;
}

QString MimeApps::appGenericName(const QString &app) const
{
    return mime_apps_items.value(app).genericName;
}

QString MimeApps::appComment(const QString &app) const
{
    return mime_apps_items.value(app).comment;
}

QString MimeApps::appPath(const QString &app) const
{
    return mime_apps_items.value(app).path;
}

QString MimeApps::appCommand(const QString &app) const
{
    return mime_apps_items.value(app).command;
}

QStringList MimeApps::appMimes(const QString &app) const
{
    return mime_apps_items.value(app).mimes;
}

void MimeApps::openFiles(const QString &app, const QStringList &files)
{
    if( !mime_apps_items.contains(app) )
        return;

    const MimeAppsItem & item = mime_apps_items.value(app);

    QString cmd;
    QStringList args;

    const QStringList & command_splits = item.command.split(" ",QString::SkipEmptyParts);

    cmd = command_splits.first();
    args = command_splits.mid(1);

    if( args.contains("%u") )
    {
        int index = args.indexOf("%u");
        foreach( const QString & f, files )
        {
            QStringList targs = args;
            targs.replace(index,f);

            QProcess::startDetached( cmd, targs );
        }
    }
    else
    if( args.contains("%U") )
    {
        int index = args.indexOf("%U");
        QStringList targs = args;
        targs.removeAt(index);

        foreach( const QString & f, files )
            targs.insert(index,f);

        QProcess::startDetached( cmd, targs );
    }
    else
    if( args.contains("%f") )
    {
        int index = args.indexOf("%f");
        foreach( const QString & f, files )
        {
            QStringList targs = args;
            targs.replace(index,f);

            QProcess::startDetached( cmd, targs );
        }
    }
    else
    if( args.contains("%F") )
    {
        int index = args.indexOf("%F");
        QStringList targs = args;
        targs.removeAt(index);

        foreach( const QString & f, files )
            targs.insert(index,f);

        QProcess::startDetached( cmd, targs );
    }
    else
    {
        foreach( const QString & f, files )
        {
            QStringList targs = args;
            targs.append(f);

            QProcess::startDetached( cmd, targs );
        }
    }
}

MimeApps::~MimeApps()
{
    delete p;
}