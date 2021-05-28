#include "appmodel.h"
#include <src/Interface/ukuichineseletter.h>
#include <QApplication>
#include <gio/gdesktopappinfo.h>

appModel::appModel()
{
    m_interFace=new UkuiMenuInterface;
    QStringList blaclist=UkuiMenuInterface::blackPathList;
    UkuiMenuInterface::appInfoVector=m_interFace->createAppInfoVector();
    UkuiMenuInterface::alphabeticVector=m_interFace->getAlphabeticClassification();
    UkuiMenuInterface::functionalVector=m_interFace->getFunctionalClassification();
    UkuiMenuInterface::allAppVector=m_interFace->getAllApp();
    qDebug()<<"blacklist  :"<<UkuiMenuInterface::blackPathList;
}

int appModel::rowCount(const QModelIndex& index) const
{
    return index.isValid() ? 0 : m_Showresult.count();
}

QVariant appModel::data(const QModelIndex &index, int role) const
{
    switch(role){
    case Qt::DisplayRole:
           return m_Showresult.at(index.row());
    case Qt::DecorationRole:
        if(index.row()<3)
            return m_Iconresult.at(index.row());
    case Qt::SizeHintRole:
        return QSize(200,46);
    case Qt::FontRole:
         return QFont("宋体",12,QFont::Medium);

        }
    return QVariant();
}

//运行对应条目的设置
void appModel::run(int index)
{
    if(UkuiMenuInterface::blackPathList.contains(m_Desktopfile.at(index)))
        return;
    GDesktopAppInfo *deskAppInfo=g_desktop_app_info_new_from_filename(m_Desktopfile.at(index).toLocal8Bit());
    bool res=static_cast<bool>(g_app_info_launch(G_APP_INFO(deskAppInfo),nullptr,nullptr,nullptr));
    g_object_unref(deskAppInfo);
}

//匹配初始化
void appModel::matchstart(const QString &source){
    sourcetext=source;
    m_Showresult.clear();
    m_Iconresult.clear();
    m_Desktopfile.clear();
    matching();
    if(sourcetext.isEmpty())
    {
         matchesChanged();
        return ;
    }

}

//将编辑框的字符串与xml文件解析出的结果正则表达式匹配
void appModel::matching(){
    if(sourcetext==NULL){
        Q_EMIT requestUpdateSignal(0);
        return;
    }
    QString appName;
    QString shouzimuName;
    QString pinyinName;
    QPixmap pixmap;
    QIcon icon;
    QImage image;
    for(int i=0;i<UkuiMenuInterface::desktopfpVector.count();i++){
        appName=m_interFace->getAppName(UkuiMenuInterface::desktopfpVector.at(i));
        pinyinName=UkuiChineseLetter::getPinyins(appName);
        shouzimuName=UkuiChineseLetter::getFirstLetters(appName);
        icon = QIcon::fromTheme(m_interFace->getAppIcon(UkuiMenuInterface::desktopfpVector.at(i)));
        pixmap = icon.pixmap(QSize(24,24));
        if(appName.contains(sourcetext)){
            m_Showresult.append(appName);
            if(UkuiMenuInterface::blackPathList.contains(UkuiMenuInterface::desktopfpVector.at(i)))
                grayIcon(pixmap);
            m_Iconresult.append(pixmap);
            m_Desktopfile.append(UkuiMenuInterface::desktopfpVector.at(i));
        }else{
            //拼音匹配
            if(shouzimuName.contains(sourcetext,Qt::CaseInsensitive)){
                m_Showresult.append(appName);
                if(UkuiMenuInterface::blackPathList.contains(UkuiMenuInterface::desktopfpVector.at(i)))
                    grayIcon(pixmap);
                m_Iconresult.append(pixmap);
                m_Desktopfile.append(UkuiMenuInterface::desktopfpVector.at(i));
            }else{
                //首字母匹配
                if(pinyinName.contains(sourcetext,Qt::CaseInsensitive)){
                     m_Showresult.append(appName);
                     if(UkuiMenuInterface::blackPathList.contains(UkuiMenuInterface::desktopfpVector.at(i)))
                         grayIcon(pixmap);
                     m_Iconresult.append(pixmap);
                     m_Desktopfile.append(UkuiMenuInterface::desktopfpVector.at(i));
                }
            }
        }

    }
    m_Showresult=m_Showresult.mid(0,3);
    m_Iconresult=m_Iconresult.mid(0,3);
    m_Desktopfile=m_Desktopfile.mid(0,3);
    qDebug()<<m_Desktopfile;
    matchesChanged();
    Q_EMIT requestUpdateSignal(m_Showresult.count());
}

//编辑栏内容改变，将model重新刷新
void appModel::matchesChanged()
{
        beginResetModel();
        endResetModel();
}

void appModel::grayIcon(QPixmap &input){
    QImage image;
    image=input.toImage();
    image = image.convertToFormat(QImage::Format_Indexed8);
    image.setColorCount(256);
    for(int i = 0; i < 256; i++)
    {
        image.setColor(i, qRgb(i, i, i));
    }
    input=QPixmap::fromImage(image);
}

