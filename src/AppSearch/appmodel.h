#ifndef APPMODEL_H
#define APPMODEL_H

#include <QObject>
#include <QTimer>
#include <QAbstractListModel>
#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QList>
#include <QProcess>
#include <QHash>
#include <QMap>
#include <QDomElement>
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QFont>
#include "src/Interface/ukuimenuinterface.h"
class appModel : public QAbstractListModel
{
    Q_OBJECT
public:
    appModel();
    int rowCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex&, int) const override;
    void run(int index);
    void matchstart(const QString &source);


private:
    void matching();
    void matchesChanged();
    void grayIcon(QPixmap &input);

    QString sourcetext;
    QStringList keylist;
    QList<QString> settinglist;
    QTimer *startmatchTimer;
    QTimer *finishtimer;
    QSettings *setting;

    QString index2;
    QList<QString> m_Showresult;
    QList<QPixmap> m_Iconresult;
    QList<QString> m_Desktopfile;
    UkuiMenuInterface *m_interFace;




Q_SIGNALS:
    /**
     * @brief Request update signal
     */
    void requestUpdateSignal(int row);


};

#endif // SETTINGMODEL_H
