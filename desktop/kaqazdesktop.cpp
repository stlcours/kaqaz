/*
    Copyright (C) 2014 Sialan Labs
    http://labs.sialan.org

    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This project is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define TOOLBAR_HEIGHT   40
#define SYNC_PBAR_HEIGHT 8

#include "kaqazdesktop.h"
#include "kaqaz.h"
#include "database.h"
#include "repository.h"
#include "backuper.h"
#include "kaqazsync.h"
#include "panelbox.h"
#include "sialantools/sialandevices.h"
#include "sialantools/sialantools.h"
#include "mimeapps.h"
#include "aboutdialog.h"
#include "papersview.h"
#include "editorview.h"
#include "editorviewmanager.h"
#include "configurepage.h"
#include "addgroupdialog.h"
#include "sialantools/sialandesktoptools.h"

#include <QToolBar>
#include <QAction>
#include <QListView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QApplication>
#include <QSplitter>
#include <QTimerEvent>
#include <QTabBar>
#include <QPaintEvent>
#include <QPainter>
#include <QProgressBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

class KaqazDesktopPrivate
{
public:
    Kaqaz *kaqaz;
    SialanDesktopTools *desktop;

    QVBoxLayout *main_layout;

    QHBoxLayout *toolbar_layout;
    QToolBar *toolbar;
    QAction *new_act;
    QAction *new_grp_act;
    QAction *conf_act;
    QAction *rsync_act;
    QAction *about_act;

    Database *db;
    Repository *repository;
    Backuper *backuper;
    KaqazSync *sync;
    SialanDevices *devices;
    SialanTools *tools;
    MimeApps *mimeApps;

    PanelBox *panel;
    PapersView *papers_view;
    EditorViewManager *editor;
    QSplitter *splitter;

    QTabBar *tabbar;
    QVBoxLayout *tabbar_layout;
    QProgressBar *sync_pbar;

    QFont font;

    int splitter_save_timer;
    int resize_save_timer;
};

KaqazDesktop::KaqazDesktop() :
    QWidget()
{
    p = new KaqazDesktopPrivate;
    p->splitter_save_timer = 0;
    p->resize_save_timer = 0;
    p->kaqaz = Kaqaz::instance();

    p->desktop = new SialanDesktopTools(this);

    QFontDatabase::addApplicationFont( p->kaqaz->resourcePath() + "/fonts/DroidKaqazSans.ttf" );

    p->font.setPointSize(10);
//    QApplication::setFont(p->font);

    resize( p->kaqaz->settings()->value("UserInterface/size",QSize(1024,600)).toSize() );

    p->main_layout = new QVBoxLayout(this);
    p->main_layout->setContentsMargins(0,0,0,0);
    p->main_layout->setSpacing(0);

    init_toolbar();
    init_mainWidget();
}

void KaqazDesktop::init_toolbar()
{
    p->new_act = new QAction( QIcon::fromTheme("document-new"), tr("Add Paper"), this );
    p->new_grp_act = new QAction( QIcon::fromTheme("document-new"), tr("Add Label"), this );
    p->rsync_act = new QAction( QIcon::fromTheme("view-refresh"), tr("Sync"), this );
    p->conf_act = new QAction( QIcon::fromTheme("configure"), tr("Configure"), this );
    p->about_act = new QAction( QIcon::fromTheme("help-about"), tr("About"), this );

    p->toolbar = new QToolBar();
    p->toolbar->addAction(p->new_act);
    p->toolbar->addAction(p->new_grp_act);
    p->toolbar->addSeparator();
    p->toolbar->addAction(p->rsync_act);
    p->toolbar->addSeparator();
    p->toolbar->addAction(p->conf_act);
    p->toolbar->addAction(p->about_act);
    p->toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    p->toolbar->setMovable(false);

    p->tabbar_layout = new QVBoxLayout();
    p->tabbar_layout->addStretch();
    p->tabbar_layout->setContentsMargins(0,0,0,0);
    p->tabbar_layout->setSpacing(0);

    QPalette palette;
    palette.setColor( QPalette::Window, p->desktop->titleBarColor() );
    palette.setColor( QPalette::WindowText, p->desktop->titleBarTextColor() );
    palette.setColor( QPalette::Button, p->desktop->titleBarColor() );
    palette.setColor( QPalette::ButtonText, p->desktop->titleBarTextColor() );

    QWidget *wgt = new QWidget();
    wgt->setFixedHeight(TOOLBAR_HEIGHT);

    p->toolbar->setPalette(palette);
    p->toolbar_layout = new QHBoxLayout(wgt);
    p->toolbar_layout->addWidget(p->toolbar);
    p->toolbar_layout->addLayout(p->tabbar_layout);
    p->toolbar_layout->setContentsMargins(0,0,0,0);
    p->toolbar_layout->setSpacing(0);

    p->main_layout->addWidget(wgt);
}

void KaqazDesktop::init_mainWidget()
{
    p->panel = new PanelBox(this);
    p->papers_view = new PapersView(this);

    p->editor = new EditorViewManager(this);

    p->tabbar_layout->addWidget(p->editor->tabBar());

    p->splitter = new QSplitter();
    p->splitter->addWidget(p->panel);
    p->splitter->addWidget(p->papers_view);
    p->splitter->addWidget(p->editor);
    p->splitter->setStyleSheet("QSplitter::handle{background: #f3f3f3}");
    p->splitter->setSizes( QList<int>()<< 213 << 273 << width()-486 );
    p->splitter->restoreState( p->kaqaz->settings()->value("UserInterface/splitter").toByteArray() );

    p->main_layout->addWidget(p->splitter);

    p->sync_pbar = new QProgressBar(this);
    p->sync_pbar->resize( p->editor->width(), SYNC_PBAR_HEIGHT );
    p->sync_pbar->move( width()-p->sync_pbar->width(), height()-SYNC_PBAR_HEIGHT );
    p->sync_pbar->setTextVisible(false);
    p->sync_pbar->setVisible(false);

    connect( p->splitter   , SIGNAL(splitterMoved(int,int))      , this          , SLOT(save_splitter())        );
    connect( p->panel      , SIGNAL(showPaperRequest(QList<int>)), p->papers_view, SLOT(showPapers(QList<int>)) );
    connect( p->papers_view, SIGNAL(paperSelected(int))          , p->editor     , SLOT(setMainPaper(int))      );
    connect( p->papers_view, SIGNAL(paperOpened(int))            , p->editor     , SLOT(addPaper(int))          );
    connect( p->new_act    , SIGNAL(triggered())                 , p->editor     , SLOT(addPaper())             );
    connect( p->new_grp_act, SIGNAL(triggered())                 , this          , SLOT(addGroup())             );
    connect( p->rsync_act  , SIGNAL(triggered())                 , this          , SLOT(refreshSync())          );
    connect( p->conf_act   , SIGNAL(triggered())                 , this          , SLOT(showConfigure())        );
    connect( p->about_act  , SIGNAL(triggered())                 , this          , SLOT(showAbout())            );
}

void KaqazDesktop::setDatabase(Database *db)
{
    p->db = db;
}

void KaqazDesktop::setRepository(Repository *rep)
{
    p->repository = rep;
}

void KaqazDesktop::setBackuper(Backuper *bkpr)
{
    p->backuper = bkpr;
}

void KaqazDesktop::setKaqazSync(KaqazSync *ksync)
{
    p->sync = ksync;

    connect( p->sync, SIGNAL(syncStarted())      , SLOT(syncStarted())       );
    connect( p->sync, SIGNAL(syncProgress(qreal)), SLOT(syncProgress(qreal)) );
    connect( p->sync, SIGNAL(syncFinished())     , SLOT(syncFinished())      );
}

void KaqazDesktop::setSialanDevices(SialanDevices *sdev)
{
    p->devices = sdev;
}

void KaqazDesktop::setSialanTools(SialanTools *stools)
{
    p->tools = stools;
}

void KaqazDesktop::setMimeApps(MimeApps *mapp)
{
    p->mimeApps = mapp;
}

bool KaqazDesktop::start()
{
    if( !p->db->password().isEmpty() )
    {
        const QString & pass = QInputDialog::getText(this, tr("Kaqaz security"), tr("Please enter password:"), QLineEdit::Password);
        const QString & md5 = p->kaqaz->passToMd5(pass);
        if( md5 != p->db->password() )
        {
            QMessageBox::critical(this, tr("Incorrect"), tr("Password is incorrect!"));
            QApplication::exit();
            return false;
        }
    }

    show();
    return true;
}

void KaqazDesktop::refresh()
{

}

void KaqazDesktop::lock()
{

}

void KaqazDesktop::addGroup()
{
    AddGroupDialog dialog(-1);
    dialog.exec();
}

void KaqazDesktop::showConfigure()
{
    ConfigurePage configure;
    configure.exec();
}

void KaqazDesktop::showAbout()
{
    AboutDialog about;
    about.exec();
}

void KaqazDesktop::save_splitter()
{
    if( p->splitter_save_timer )
        killTimer(p->splitter_save_timer);

    p->splitter_save_timer = startTimer(500);
}

void KaqazDesktop::syncStarted()
{
    p->sync_pbar->setVisible(true);
}

void KaqazDesktop::syncProgress(qreal val)
{
    p->sync_pbar->setVisible(true);
    p->sync_pbar->setValue( val*100 );
}

void KaqazDesktop::syncFinished()
{
    p->sync_pbar->setVisible(false);
}

void KaqazDesktop::refreshSync()
{
    if( !p->sync->tokenAvailable() )
        return;

    p->sync->refreshForce();
    p->sync_pbar->setVisible(true);
    p->sync_pbar->setValue(0);
}

void KaqazDesktop::timerEvent(QTimerEvent *e)
{
    if( e->timerId() == p->splitter_save_timer )
    {
        killTimer(p->splitter_save_timer);
        p->splitter_save_timer = 0;

        p->kaqaz->settings()->setValue("UserInterface/splitter", p->splitter->saveState() );
    }
    else
    if( e->timerId() == p->resize_save_timer )
    {
        killTimer(p->resize_save_timer);
        p->resize_save_timer = 0;

        p->kaqaz->settings()->setValue("UserInterface/size", size() );
    }

    QWidget::timerEvent(e);
}

void KaqazDesktop::resizeEvent(QResizeEvent *e)
{
    if( p->resize_save_timer )
        killTimer(p->resize_save_timer);

    p->resize_save_timer = startTimer(500);

    p->sync_pbar->resize( p->editor->width(), SYNC_PBAR_HEIGHT );
    p->sync_pbar->move( width()-p->sync_pbar->width(), height()-SYNC_PBAR_HEIGHT );

    QWidget::resizeEvent(e);
}

void KaqazDesktop::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter painter(this);
    painter.fillRect(0,0,width(),TOOLBAR_HEIGHT,p->desktop->titleBarColor());
    painter.fillRect(0,TOOLBAR_HEIGHT,width(),height()-TOOLBAR_HEIGHT,palette().window());
}

KaqazDesktop::~KaqazDesktop()
{
    delete p;
}