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

#include "searchresultwidget.h"
#include "src/Style/style.h"
#include <QDebug>
#include <QHeaderView>

SearchResultWidget::SearchResultWidget(QWidget *parent) :
    QWidget(parent)
{
    initUi();
    installEventFilter(this);
}

SearchResultWidget::~SearchResultWidget()
{
    delete m_ukuiMenuInterface;
}

void SearchResultWidget::initUi()
{
    m_listLayout = new QVBoxLayout;
    m_headLabel = new HeadLabel(this);
    m_Button = new MoreButton(this);
    m_Button->setText("在应用商店中搜索更多");
    m_headLabel->setText(tr("App"));
    m_headLabel->setFixedHeight(48);
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    this->setFixedSize(Style::defaultMainViewWidWidth,200);
//    this->setStyleSheet("background:white;");

    m_listView=new ListView(this,this->width()-6,this->height()-6,3);
    m_listView->setGeometry(QRect(6,0,this->width()-6,this->height()-6));
    m_listView->show();

    m_data.clear();
    m_listView->addData(m_data);//添加应用数据
    m_ukuiMenuInterface=new UkuiMenuInterface;

    connect(m_listView,&ListView::sendItemClickedSignal,this,&SearchResultWidget::execApplication);
    connect(m_Button,&QPushButton::clicked,this,[=](){
        QProcess *process =new QProcess(this);
        process->startDetached("ubuntu-kylin-software-center");
    });
    //监听输入框的改变，刷新界面
    QDBusConnection::sessionBus().connect(QString(), QString("/lineEdit/textChanged"), "org.ukui.search.inputbox", "InputBoxTextChanged", this, SLOT(appTextRefresh(QString)));


    //跑一个线程执行应用搜索
    m_searchAppThread=new SearchAppThread;

    connect(this,&SearchResultWidget::sendSearchKeyword,
            m_searchAppThread,&SearchAppThread::recvSearchKeyword);

    connect(m_searchAppThread,&SearchAppThread::sendSearchResult,
            this,&SearchResultWidget::recvSearchResult);


    m_listLayout->addWidget(m_headLabel);
    m_listLayout->addWidget(m_listView);
    m_listLayout->addWidget(m_Button);
    this->setLayout(m_listLayout);
    this->setVisible(false);
}

/**
 * 执行应用程序
 */
void SearchResultWidget::execApplication(QStringList arg)
{
    QStringList blaclist=UkuiMenuInterface::blackPathList;
    QString desktopfp=arg.at(0);
    if(!blaclist.contains(desktopfp)){
    GDesktopAppInfo * desktopAppInfo=g_desktop_app_info_new_from_filename(desktopfp.toLocal8Bit().data());
    g_app_info_launch(G_APP_INFO(desktopAppInfo),nullptr, nullptr, nullptr);
    g_object_unref(desktopAppInfo);
    }
}

void SearchResultWidget::updateAppListView(QVector<QStringList> arg)
{
    m_data.clear();
    Q_FOREACH(QStringList appinfo,arg)
        m_data.append(QStringList()<<appinfo.at(0)<<"1");
    Q_EMIT changeAppNum(m_data.count());
    m_listView->updateData(m_data);
    //根据获取的应用数量，刷新界面高度
    if(m_data.size()<=0){
        this->setVisible(false);
    } else if(m_data.size()<=2){
        this->setVisible(true);
        this->setFixedSize(Style::defaultMainViewWidWidth,m_data.size()*54+60);
        m_Button->hide();
    } else {
        this->setVisible(true);
        this->setFixedSize(Style::defaultMainViewWidWidth,3.3*56+46+m_Button->height());
        m_Button->show();
    }

}

void SearchResultWidget::appTextRefresh(QString mSearchText)
{
    m_searchAppThread->start();
    Q_EMIT sendSearchKeyword(mSearchText);
}
void SearchResultWidget::recvSearchResult(QVector<QStringList> arg)
{
    m_searchAppThread->quit();
    updateAppListView(arg);
}


void SearchResultWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QRect rect = this->rect();
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setBrush(qApp->palette().color(QPalette::Base));
//    p.setBrush(QBrush(QColor(255,255,255)));
    p.setOpacity(1);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect,12,12);
    QWidget::paintEvent(e);
}

bool SearchResultWidget::eventFilter(QObject *watched, QEvent *event){
    if(watched==this){
        if(event->type()==QEvent::MouseButtonRelease){
            return true;
        }
    }
    return false;
}

void SearchResultWidget::fristSelect(){
    m_listView->setCurrentIndex(m_listView->listmodel->index(0,0));
}
