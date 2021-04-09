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

#ifndef LISTVIEW_H
#define LISTVIEW_H
#include <QListView>
#include <QStandardItem>
#include <QEvent>
#include <QScrollBar>
#include <QStandardItemModel>
#include "src/Style/style.h"
#include <QSettings>
#include <QEvent>
#include <QKeyEvent>
#include "src/Interface/ukuimenuinterface.h"
#include "itemdelegate.h"
#include <QGSettings>
#include <QTreeView>

class ListView : public QTreeView
{
    Q_OBJECT
public:
    ListView(QWidget *parent, int width, int height, int module);
    ~ListView();

    void addData(QVector<QStringList> data);
    void updateData(QVector<QStringList> data);
    void setsize(int x,int y);
    QStandardItemModel* listmodel=nullptr;

protected:
    void initWidget();
    void enterEvent(QEvent* e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent* e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) override;

private:
//    QStandardItemModel* listmodel=nullptr;
    int w=0;
    int h=0;
    int module=0;
    UkuiMenuInterface* pUkuiMenuInterface=nullptr;
    QStringList header;

    QSettings *setting=nullptr;
    QVector<QStringList> data;
    ItemDelegate* m_delegate=nullptr;


    //控制style的gsetting
    QGSettings *style_settings;
    //用来判断style
    QStringList stylelist;

    void styleChange();

private Q_SLOTS:
    void onClicked(QModelIndex index);//点击item
Q_SIGNALS:
    void sendItemClickedSignal(QStringList arg);//发送item点击信号
    void sendHideMainWindowSignal();
    void sendUpdateAppListSignal();
};

#endif // LISTVIEW_H
