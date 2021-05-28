#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QtDBus/QtDBus>
#include <QDebug>
#include <QString>
#include "appmodel.h"
#include "appview.h"
#include "../Interface/morebutton.h"
#include "../Interface/headlabel.h"
#include <QPainter>


class AppWidget : public QWidget
{
    Q_OBJECT
public:
     AppWidget();
     appview *settingView;
     appModel *m_settingmodel;//设置model
     QVBoxLayout *m_listLayout;

private:
     void initSettingsearchUI();
     void paintEvent(QPaintEvent *e);
     bool eventFilter(QObject *watched, QEvent *event);

private:
     MoreButton *m_Button;
     HeadLabel *m_Headlabel;

private Q_SLOTS:
     void settingTextRefresh(QString);
     void recvSettingSearchResult(int row);

public Q_SLOTS:
     void fristSelect();

Q_SIGNALS:
//     void sendSearchKeyword(QString arg);


};

#endif // SETTINGWIDGET_H
