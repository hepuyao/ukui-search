/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "mainviewwidget.h"
#include "src/Style/style.h"
#include <QSvgRenderer>
#include <QPainter>
#include <libbamf/bamf-matcher.h>
#include <syslog.h>
#include <QDebug>

MainViewWidget::MainViewWidget(QWidget *parent) :
    QWidget(parent)
{
    fileView = new QTreeView(this);//文档搜索treeview
    settingView  = new QTreeView(this);//控制面板搜索treeview
    settingmodel= new settingModel;
    m_filemodel = new filemodel;
    initUi();
}

MainViewWidget::~MainViewWidget()
{

    delete m_ukuiMenuInterface;
    delete m_fileWatcher;
    delete m_directoryChangedThread;
    delete m_animation;
    delete m_searchAppThread;

}

void MainViewWidget::initUi()
{
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet("border:0px;background:transparent;");

    QVBoxLayout* mainLayout=new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    m_topWidget=new QWidget;//搜索框背景
    m_topWidget->setStyleSheet("border:0px;background:transparent;");

    m_verticalSpacer=new QSpacerItem(20,80, QSizePolicy::Fixed, QSizePolicy::Expanding);
    mainLayout->addWidget(m_topWidget);
    mainLayout->addItem(m_verticalSpacer);
    this->setLayout(mainLayout);

    this->setFocusPolicy(Qt::NoFocus);

//    m_commonUseWid=new CommonUseWidget;
//    m_fullCommonUseWid=new FullCommonUseWidget;
//    m_letterWid=new LetterWidget;
//    m_functionWid=new FunctionWidget;
//    m_fullLetterWid=new FullLetterWidget;
//    m_fullFunctionWid=new FullFunctionWidget;
//    m_fullSearchResultWid=new FullSearchResultWidget;
    m_searchResultWid=new SearchResultWidget;

    m_ukuiMenuInterface=new UkuiMenuInterface;

    //监控.desktop文件目录
    m_fileWatcher=new QFileSystemWatcher;
    m_fileWatcher->addPaths(QStringList()<<"/usr/share/applications"<<QDir::homePath()+"/.local/share/applications/");
    connect(m_fileWatcher,&QFileSystemWatcher::directoryChanged,this,&MainViewWidget::directoryChangedSlot);
    m_directoryChangedThread=new DirectoryChangedThread;
    connect(this,&MainViewWidget::sendDirectoryPath,m_directoryChangedThread,&DirectoryChangedThread::recvDirectoryPath);
    connect(m_directoryChangedThread,&DirectoryChangedThread::requestUpdateSignal,this,&MainViewWidget::requestUpdateSlot);


    //发送隐藏主界面信号

    connect(m_searchResultWid,&SearchResultWidget::sendHideMainWindowSignal,this,&MainViewWidget::sendHideMainWindowSignal);

    addTopControl();
    //加载默认视图

    this->setFixedSize(Style::defaultMainViewWidWidth,Style::minh);
    m_topWidget->setFixedSize(Style::defaultMainViewWidWidth,Style::defaultTopWidHeight);
    m_topLayout->setContentsMargins(0,0,0,0);
    m_topLayout->setAlignment(m_queryLineEdit,Qt::AlignCenter);
    m_queryLineEdit->setFixedSize(Style::defaultQueryLineEditWidth,Style::defaultQueryLineEditHeight);
    m_queryText->adjustSize();
    m_queryWid->setGeometry(QRect((m_queryLineEdit->width()-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                  m_queryIcon->width()+m_queryText->width()+10,Style::QueryLineEditHeight));
    m_queryWid->show();

    mainLayout->addWidget(m_searchResultWid);



}

/**
 * 添加顶部控件
 */
void MainViewWidget::addTopControl()
{
    m_topLayout=new QHBoxLayout;
    m_topLayout->setSpacing(0);
    m_queryLineEdit=new QLineEdit;
    char style[100];
    sprintf(style, "QLineEdit{border:0px;background-color:%s;border-radius:4px;}",QueryLineEditBackground);
//    m_queryLineEdit->setStyleSheet(style);
    m_queryLineEdit->setStyleSheet("border:0px;background:white;"
                                   "color:black;"
                                   "border:2px rgb(41, 237, 215);"
                                   "border-radius:10px;"
                                   "padding:2px 4px;");

    m_topLayout->addWidget(m_queryLineEdit);
    m_topWidget->setLayout(m_topLayout);

    initQueryLineEdit();

}

/**
 * 添加搜索框
 */
void MainViewWidget::initQueryLineEdit()
{
    m_queryWid=new QWidget;
//    m_queryWid->setParent(m_queryLineEdit);
    m_queryWid->setFocusPolicy(Qt::NoFocus);
    m_queryWid->setStyleSheet("border:0px;background:transparent");
    QHBoxLayout* queryWidLayout=new QHBoxLayout;
    queryWidLayout->setContentsMargins(5,0,0,0);
    queryWidLayout->setSpacing(5);
    m_queryWid->setLayout(queryWidLayout);
    QPixmap pixmap;
    pixmap.load(QString(":/data/img/mainviewwidget/search.png"));
    m_queryIcon=new QLabel;
    m_queryIcon->setStyleSheet("background:transparent");
    m_queryIcon->setFixedSize(pixmap.size());
    m_queryIcon->setPixmap(pixmap);
    m_queryText=new QLabel;
    m_queryText->setText(tr("Search"));
    m_queryText->setStyleSheet("background:transparent;color:#626c6e;");
    m_queryText->adjustSize();
    queryWidLayout->addWidget(m_queryIcon);
    queryWidLayout->addWidget(m_queryText);
    m_queryLineEdit->setFocusPolicy(Qt::ClickFocus);
    m_queryLineEdit->installEventFilter(this);
    m_queryLineEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QAction *searchAction = new QAction(m_queryLineEdit);
    searchAction->setIcon(QIcon(":/data/img/mainviewwidget/search.png"));
    m_queryLineEdit->addAction(searchAction,QLineEdit::LeadingPosition );


    m_animation= new QPropertyAnimation(m_queryWid,"geometry");
    m_animation->setDuration(100);
    connect(m_animation,&QPropertyAnimation::finished,this,&MainViewWidget::animationFinishedSlot);

    m_searchAppThread=new SearchAppThread;
    connect(this,&MainViewWidget::sendSearchKeyword,
            m_searchAppThread,&SearchAppThread::recvSearchKeyword);
    connect(m_searchAppThread,&SearchAppThread::sendSearchResult,
            this,&MainViewWidget::recvSearchResult);
    connect(m_queryLineEdit, &QLineEdit::textChanged, this, &MainViewWidget::searchAppSlot);

    connect(m_queryLineEdit,&QLineEdit::textChanged,settingmodel,[=](const QString &search){
                settingmodel->matchstart(search);

    });

    connect(m_queryLineEdit,&QLineEdit::textChanged,m_filemodel,[=](const QString &search){
                m_filemodel->matchstart(search);
           qDebug()<<"ok";
    });


    connect(settingView,&QTreeView::clicked,this,[=](){
        settingmodel->run(settingView->currentIndex().row());

    });

    connect(fileView,&QTreeView::clicked,this,[=](){
        m_filemodel->run(fileView->currentIndex().row());
    });

}

bool MainViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==m_queryLineEdit)
    {
        if(event->type()==QEvent::FocusIn)
        {
             char style[200];
             sprintf(style, "QLineEdit{border:1px solid %s;background-color:%s;border-radius:4px;color:#ffffff;}",
                     QueryLineEditClickedBorder,QueryLineEditClickedBackground);
//             m_queryLineEdit->setStyleSheet(style);
             if(!m_queryLineEdit->text().isEmpty())
             {
                 if(m_searchKeyWords.isEmpty())
                    searchAppSlot(m_queryLineEdit->text());
             }
             else
             {
                 m_animation->stop();
                 m_animation->setStartValue(QRect((m_queryLineEdit->width()-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                                m_queryIcon->width()+m_queryText->width()+10,Style::QueryLineEditHeight));
                 m_animation->setEndValue(QRect(0,0,
                                               m_queryIcon->width()+5,Style::QueryLineEditHeight));
                 m_animation->setEasingCurve(QEasingCurve::OutQuad);
                 m_animation->start();
//                 m_queryLineEdit->setReadOnly(true);
                 m_queryLineEdit->setTextMargins(-5,1,0,1);
             }
             m_isSearching=true;
        }
        else if(event->type()==QEvent::FocusOut)
        {
            m_searchKeyWords.clear();
            if(m_queryLineEdit->text().isEmpty())
            {
                if(m_isSearching)
                {
                    char style[100];
                    sprintf(style, "QLineEdit{border:0px;background-color:%s;border-radius:4px;}",QueryLineEditBackground);
                    m_animation->stop();
//                    m_queryLineEdit->setStyleSheet(style);
                    m_queryText->adjustSize();
                    m_animation->setStartValue(QRect(0,0,
                                                    m_queryIcon->width()+5,Style::QueryLineEditHeight));
                    m_animation->setEndValue(QRect((m_queryLineEdit->width()-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                                 m_queryIcon->width()+m_queryText->width()+10,Style::QueryLineEditHeight));
                    m_animation->setEasingCurve(QEasingCurve::InQuad);
                    m_animation->start();
                }
            }
            else {
                char style[100];
                sprintf(style, "QLineEdit{border:0px;background-color:%s;border-radius:4px;color:#ffffff;}",
                        QueryLineEditBackground);
//                m_queryLineEdit->setStyleSheet(style);
            }
            m_isSearching=false;
        }
    }

     return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
}

void MainViewWidget::setLineEditFocus(QString arg)
{
    if(!m_queryLineEdit->hasFocus())
    {
        m_searchKeyWords=arg;
        m_queryLineEdit->setFocus();
        if(!m_queryLineEdit->text().isEmpty())
            m_queryLineEdit->setText(arg);
    }
}

/**
 * 搜索程序和文件槽函数
 */
void MainViewWidget::searchAppSlot(QString arg)
{

    Q_EMIT sendSearchKeyword(arg);
    m_searchAppThread->start();

}

void MainViewWidget::recvSearchResult(QVector<QStringList> arg)
{
    m_searchAppThread->quit();
    m_searchResultWid->updateAppListView(arg);
}

void MainViewWidget::animationFinishedSlot()
{
    if(m_isSearching)
    {
        m_queryWid->layout()->removeWidget(m_queryText);
        m_queryText->setParent(nullptr);
        m_queryLineEdit->setTextMargins(20,1,0,1);
        if(!m_searchKeyWords.isEmpty())
        {
            m_queryLineEdit->setText(m_searchKeyWords);
            m_searchKeyWords.clear();
        }
    }
    else
        m_queryWid->layout()->addWidget(m_queryText);
}

/**
 * 加载默认主视图
 */
void MainViewWidget::loadMinMainView()
{
    this->setFixedSize(Style::defaultMainViewWidWidth,Style::minh);
    m_topWidget->setFixedSize(Style::defaultMainViewWidWidth,Style::defaultTopWidHeight);
    m_topLayout->setContentsMargins(0,0,0,0);
    m_topLayout->setAlignment(m_queryLineEdit,Qt::AlignCenter);
    m_queryLineEdit->setFixedSize(Style::defaultQueryLineEditWidth,Style::defaultQueryLineEditHeight);
    if(m_queryLineEdit->text().isEmpty())
    {
        if(m_queryWid->layout()->count()==1)
            m_queryWid->layout()->addWidget(m_queryText);
        m_queryText->adjustSize();
        m_queryWid->setGeometry(QRect((m_queryLineEdit->width()-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
                                      m_queryIcon->width()+m_queryText->width()+10,Style::QueryLineEditHeight));
        m_queryWid->show();
    }

    QLayoutItem* child;
    if((child=this->layout()->takeAt(1))!=nullptr)
    {
        QWidget* childWid=child->widget();
        if(childWid!=nullptr)
        {
            this->layout()->removeWidget(childWid);
            childWid->setParent(nullptr);
        }
    }
    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
    layout->addWidget(m_searchResultWid);
//    m_searchResultWid->setStyleSheet("QFrame {border:1px solid yellow;}"); //测试边界线


    settingView->setModel(settingmodel);
     fileView->setModel(m_filemodel);


    fileView->setStyleSheet("color:black;"
                               " background-color : white;"
                               " border: 0px none;border-radius: 10px;"

                              );

    settingView->setStyleSheet("color:black;"
                               " background-color : white;"
                               " border: 0px none;border-radius: 10px;"

                              );
// fileView->setStyleSheet(QString::fromLocal8Bit("QTreeView { background-color : white; border: 0px none;border-radius: 10px;}")) ;




    layout->addItem(new QSpacerItem(15, 4));
    layout->addWidget(fileView);
    layout->addItem(new QSpacerItem(15, 4));
    layout->addWidget(settingView);

}

/**
 * 加载全屏主视图
 */
void MainViewWidget::loadMaxMainView()
{
//    this->setFixedSize(Style::MainViewWidWidth,
//                       Style::heightavailable);
//    m_topWidget->setFixedSize(this->width(),Style::TopWidgetHeight);
//    m_queryLineEdit->setFixedSize(Style::QueryLineEditWidth,Style::QueryLineEditHeight);

//    m_topLayout->setContentsMargins((m_topWidget->width()-Style::LeftWidWidth-m_queryLineEdit->width())/2+Style::LeftWidWidth,
//                                  0,
//                                  (m_topWidget->width()-Style::LeftWidWidth-m_queryLineEdit->width())/2,
//                                  0);

//    if(m_queryLineEdit->text().isEmpty())
//    {
//        if(m_queryWid->layout()->count()==1)
//            m_queryWid->layout()->addWidget(m_queryText);
//        m_queryText->adjustSize();
//        m_queryWid->setGeometry(QRect((m_queryLineEdit->width()-(m_queryIcon->width()+m_queryText->width()+10))/2,0,
//                                      m_queryIcon->width()+m_queryText->width()+10,Style::QueryLineEditHeight));
//        m_queryWid->show();
//    }

//    if(m_widgetState==0)
//    {
//        QLayoutItem* child;
//        if((child=this->layout()->takeAt(1))!=nullptr)
//        {
//            QWidget* childWid=child->widget();
//            if(childWid!=nullptr)
//            {
//                this->layout()->removeWidget(childWid);
//                childWid->setParent(nullptr);
//            }
//        }
//        QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
//        layout->insertWidget(1,m_fullSearchResultWid);
//    }
//    else if(m_widgetState==1)
//        loadFullCommonUseWidget();
//    else if(m_widgetState==2)
//        loadFullLetterWidget();
//    else if(m_widgetState==3)
//        loadFullFunctionWidget();
//    m_isFullScreen=true;
//    m_isHiden=false;
}

void MainViewWidget::resizeControl()
{
    if(m_isFullScreen)
    {
//        if(m_widgetState==2)
//            m_fullLetterWid->setLetterBtnGeometry();
//        if(m_widgetState==3)
//            m_fullFunctionWid->setFunctionBtnGeometry();

        this->setFixedSize(Style::MainViewWidWidth,
                           Style::heightavailable);
        m_topWidget->setFixedSize(this->width(),Style::TopWidgetHeight);
        m_queryLineEdit->setFixedSize(Style::QueryLineEditWidth,Style::QueryLineEditHeight);

        m_topLayout->setContentsMargins((m_topWidget->width()-Style::LeftWidWidth-m_queryLineEdit->width())/2+Style::LeftWidWidth,
                                      0,
                                      (m_topWidget->width()-Style::LeftWidWidth-m_queryLineEdit->width())/2,
                                      0);
    }
    else
    {
        this->setFixedSize(Style::defaultMainViewWidWidth,Style::minh);
        m_topWidget->setFixedSize(Style::defaultMainViewWidWidth,Style::defaultTopWidHeight);
        m_topLayout->setContentsMargins(0,0,0,0);
        m_topLayout->setAlignment(m_queryLineEdit,Qt::AlignCenter);
    }

}

/**
 * 加载常用分类界面
 */
void MainViewWidget::loadCommonUseWidget()
{
    m_fullCommonUseWid->widgetMakeZero();
    m_letterWid->widgetMakeZero();
    m_fullLetterWid->widgetMakeZero();
    m_functionWid->widgetMakeZero();
    m_fullFunctionWid->widgetMakeZero();
    QLayoutItem *child;
    if((child = this->layout()->takeAt(1)) != nullptr) {
        QWidget* childwid=child->widget();
        if(childwid!=nullptr)
        {
            this->layout()->removeWidget(childwid);
            childwid->setParent(nullptr);
        }

    }
    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
    layout->insertWidget(1,m_commonUseWid);
    m_commonUseWid->updateListView();
    m_widgetState=1;
    m_saveCurrentWidState=1;
}

/**
 * 加载字母分类界面
 */
void MainViewWidget::loadLetterWidget()
{
    m_commonUseWid->widgetMakeZero();
    m_fullCommonUseWid->widgetMakeZero();
    m_fullLetterWid->widgetMakeZero();
    m_functionWid->widgetMakeZero();
    m_fullFunctionWid->widgetMakeZero();
    QLayoutItem *child;
    if((child = this->layout()->takeAt(1)) != nullptr) {
        QWidget* childwid=child->widget();
        if(childwid!=nullptr)
        {
            this->layout()->removeWidget(childwid);
            childwid->setParent(nullptr);
        }

    }
    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
    layout->insertWidget(1,m_letterWid);
    m_widgetState=2;
    m_saveCurrentWidState=2;
}

/**
 * 加载功能分类界面
 */
void MainViewWidget::loadFunctionWidget()
{
    m_commonUseWid->widgetMakeZero();
    m_fullCommonUseWid->widgetMakeZero();
    m_letterWid->widgetMakeZero();
    m_fullLetterWid->widgetMakeZero();
    m_fullFunctionWid->widgetMakeZero();
    QLayoutItem *child;
    if((child = this->layout()->takeAt(1)) != nullptr) {
        QWidget* childwid=child->widget();
        if(childwid!=nullptr)
        {
            this->layout()->removeWidget(childwid);
            childwid->setParent(nullptr);
        }

    }
    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
    layout->insertWidget(1,m_functionWid);
    m_widgetState=3;
    m_saveCurrentWidState=3;
}


/**
 * 加载全屏常用分类界面
 */
void MainViewWidget::loadFullCommonUseWidget()
{
    m_commonUseWid->widgetMakeZero();
    m_letterWid->widgetMakeZero();
    m_fullLetterWid->widgetMakeZero();
    m_functionWid->widgetMakeZero();
    m_fullFunctionWid->widgetMakeZero();
    QLayoutItem *child;
    if((child = this->layout()->takeAt(1)) != nullptr) {
        QWidget* childwid=child->widget();
        if(childwid!=nullptr)
        {
            this->layout()->removeWidget(childwid);
            childwid->setParent(nullptr);
        }

    }
    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
    layout->insertWidget(1,m_fullCommonUseWid);
    m_fullCommonUseWid->updateListView();
    m_widgetState=1;
    m_saveCurrentWidState=1;
}

/**
 * 加载全屏字母分类界面
 */
void MainViewWidget::loadFullLetterWidget()
{
    m_commonUseWid->widgetMakeZero();
    m_fullCommonUseWid->widgetMakeZero();
    m_letterWid->widgetMakeZero();
    m_functionWid->widgetMakeZero();
    m_fullFunctionWid->widgetMakeZero();
    QLayoutItem *child;
    if((child = this->layout()->takeAt(1)) != nullptr) {
        QWidget* childwid=child->widget();
        if(childwid!=nullptr)
        {
            this->layout()->removeWidget(childwid);
            childwid->setParent(nullptr);
        }
    }
    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
    layout->insertWidget(1,m_fullLetterWid);
    if(!m_isFullScreen || (m_isFullScreen && m_saveCurrentWidState!=2))
        m_fullLetterWid->enterAnimation();
    m_widgetState=2;
    m_saveCurrentWidState=2;
}

/**
 * 加载全屏功能分类界面
 */
void MainViewWidget::loadFullFunctionWidget()
{
    m_commonUseWid->widgetMakeZero();
    m_fullCommonUseWid->widgetMakeZero();
    m_letterWid->widgetMakeZero();
    m_fullLetterWid->widgetMakeZero();
    m_functionWid->widgetMakeZero();
    QLayoutItem *child;
    if((child = this->layout()->takeAt(1)) != nullptr) {
        QWidget* childwid=child->widget();
        if(childwid!=nullptr)
        {
            this->layout()->removeWidget(childwid);
            childwid->setParent(nullptr);
        }

    }
    QVBoxLayout *layout=qobject_cast<QVBoxLayout*>(this->layout());
    layout->insertWidget(1,m_fullFunctionWid);
    if(!m_isFullScreen || (m_isFullScreen && m_saveCurrentWidState!=3))
        m_fullFunctionWid->enterAnimation();
    m_widgetState=3;
    m_saveCurrentWidState=3;
}

/**
 * 进程开启监控槽函数
 */
void MainViewWidget::ViewOpenedSlot(QDBusMessage msg)
{
    QString path=msg.arguments().at(0).value<QString>();
    QString type=msg.arguments().at(1).value<QString>();
    if(QString::compare(type,"application")==0)
    {
        QDBusInterface ifaceapp("org.ayatana.bamf",path,
                                "org.ayatana.bamf.application",QDBusConnection::sessionBus());
        QDBusReply<QString> replyapp =ifaceapp.call("DesktopFile");
        QString desktopfp=replyapp.value();
        QStringList desktopfpList=m_ukuiMenuInterface->getDesktopFilePath();
        if(desktopfpList.contains(desktopfp))
        {
            QFileInfo fileInfo(desktopfp);
            QString desktopfn=fileInfo.fileName();

            QString dateTimeKey;
            dateTimeKey.clear();
            if(!desktopfn.isEmpty())
            {
                m_setting->beginGroup("lockapplication");
                bool ret=m_setting->contains(desktopfn);
                m_setting->endGroup();
                if(!ret)
                {
                    m_setting->beginGroup("application");
                    m_setting->setValue(desktopfn,m_setting->value(desktopfn).toInt()+1);
                    dateTimeKey=desktopfn;
                    m_setting->sync();
                    m_setting->endGroup();
                }
            }

            if(!dateTimeKey.isEmpty())
            {
                QDateTime dt=QDateTime::currentDateTime();
                int datetime=dt.toTime_t();
                m_setting->beginGroup("datetime");
                m_setting->setValue(dateTimeKey,datetime);
                m_setting->sync();
                m_setting->endGroup();
            }
        }
    }
}

/**
 * desktop文件目录改变信号槽
 */
void MainViewWidget::directoryChangedSlot()
{
    Q_EMIT sendDirectoryPath(QString("/usr/share/applications"));
    m_directoryChangedThread->start();

//    QStringList desktopfpList=pUkuiMenuInterface->getDesktopFilePath();
//    if(desktopfpList.size() > UkuiMenuInterface::desktopfpVector.size())//有新的应用安装
//    {
//        m_setting->beginGroup("recentapp");
//        for(int i=0;i<desktopfpList.count();i++)
//        {
//            if(!UkuiMenuInterface::desktopfpVector.contains(desktopfpList.at(i)))
//            {
//                //获取当前时间戳
//                QDateTime dt=QDateTime::currentDateTime();
//                int datetime=dt.toTime_t();
////                QString appname=pUkuiMenuInterface->getAppName(desktopfpList.at(i));
//                QFileInfo fileInfo(desktopfpList.at(i));
//                QString desktopfn=fileInfo.fileName();
//                m_setting->setValue(desktopfn,datetime);
//                qDebug()<<"安装:"<<desktopfn;
//                break;
//            }

//        }
//        m_setting->endGroup();
//        UkuiMenuInterface::appInfoVector.clear();
//        UkuiMenuInterface::alphabeticVector.clear();
//        UkuiMenuInterface::functionalVector.clear();
//        UkuiMenuInterface::commonUseVector.clear();
//        UkuiMenuInterface::appInfoVector=pUkuiMenuInterface->createAppInfoVector();
//        UkuiMenuInterface::alphabeticVector=pUkuiMenuInterface->getAlphabeticClassification();
//        UkuiMenuInterface::functionalVector=pUkuiMenuInterface->getFunctionalClassification();
//        UkuiMenuInterface::commonUseVector=pUkuiMenuInterface->getCommonUseApp();
//        Q_EMIT directoryChangedSignal();
//    }
//    else//软件卸载
//    {
//        for(int i=0;i<UkuiMenuInterface::desktopfpVector.size();i++)
//        {
//            if(!desktopfpList.contains(UkuiMenuInterface::desktopfpVector.at(i)))
//            {
//                QString desktopfp=UkuiMenuInterface::appInfoVector.at(i).at(0);
//                QFileInfo fileInfo(desktopfp);
//                QString desktopfn=fileInfo.fileName();
//                m_setting->beginGroup("lockapplication");
//                m_setting->remove(desktopfn);
//                m_setting->sync();
//                m_setting->endGroup();
//                m_setting->beginGroup("application");
//                m_setting->remove(desktopfn);
//                m_setting->sync();
//                m_setting->endGroup();
//                m_setting->beginGroup("datetime");
//                m_setting->remove(desktopfn);
//                m_setting->sync();
//                m_setting->endGroup();
//                m_setting->beginGroup("recentapp");
//                m_setting->remove(desktopfn);
//                m_setting->sync();
//                m_setting->endGroup();
//                qDebug()<<"卸载:"<<desktopfn;
//                break;
//            }
//        }
//        UkuiMenuInterface::appInfoVector.clear();
//        UkuiMenuInterface::alphabeticVector.clear();
//        UkuiMenuInterface::functionalVector.clear();
//        UkuiMenuInterface::commonUseVector.clear();
//        UkuiMenuInterface::appInfoVector=pUkuiMenuInterface->createAppInfoVector();
//        UkuiMenuInterface::alphabeticVector=pUkuiMenuInterface->getAlphabeticClassification();
//        UkuiMenuInterface::functionalVector=pUkuiMenuInterface->getFunctionalClassification();
//        UkuiMenuInterface::commonUseVector=pUkuiMenuInterface->getCommonUseApp();
//        Q_EMIT directoryChangedSignal();
//    }
}

void MainViewWidget::requestUpdateSlot()
{
    m_directoryChangedThread->quit();
    Q_EMIT directoryChangedSignal();
}

void MainViewWidget::iconThemeChangeSlot(QString key)
{
    if(key=="iconThemeName" || key=="icon-theme-name")
        Q_EMIT directoryChangedSignal();
}

void MainViewWidget::repaintWidget()
{
    this->setMinimumSize(Style::minw,Style::minh);
//    m_commonUseWid->repaintWidget();
//    m_fullCommonUseWid->repaintWidget();
//    m_letterWid->repaintWidget();
//    m_fullLetterWid->repaintWidget();
//    m_functionWid->repaintWidget();
//    m_fullFunctionWid->repaintWidget();
    m_searchResultWid->repaintWidget();
//    m_fullSearchResultWid->repaintWidget();
}

void MainViewWidget::widgetMakeZero()
{

    m_queryLineEdit->clear();
    m_queryLineEdit->clearFocus();
    char style[100];
    sprintf(style, "QLineEdit{border:0px;background-color:%s;border-radius:2px;}",QueryLineEditBackground);
    m_queryLineEdit->setStyleSheet(style);
    m_queryLineEdit->setTextMargins(0,1,0,1);
}

void MainViewWidget::moveScrollBar(int type)
{

}

