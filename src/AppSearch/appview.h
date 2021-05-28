#ifndef APPVIEW_H
#define APPVIEW_H

#include <QTreeView>
#include "../Style/style.h"
#include <QDebug>
#include <QListView>
class appview : public QTreeView
{
public:
    appview();
    ~appview();

public Q_SLOTS:
    void changesize(int row);

private:
    //控制style的gsetting
    QGSettings *style_settings;
    //用来判断style
    QStringList stylelist;
    void styleChange();

};

#endif // SETTINGVIEW_H
