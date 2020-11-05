#include "settingmodel.h"


#define XML_Source QString::fromLocal8Bit("ChineseFunc")
#define XML_Title  QString::fromLocal8Bit("ChinesePlugin")

settingModel::settingModel():
    startmatchTimer(new QTimer(this))
{

    startmatchTimer->setSingleShot(true);
    startmatchTimer->setInterval(10);
    setting = new QSettings(QString::fromLocal8Bit(":/tests/assets/search.conf"),QSettings::IniFormat);
    keylist<<setting->allKeys();
    for(int i = 0; i< keylist.size();++i)
    {
        settinglist.append(setting->value(keylist.at(i)).toString());
    }
    connect(startmatchTimer,&QTimer::timeout,this,[=](){
        matching();
    });
    XmlElement();
}

int settingModel::rowCount(const QModelIndex& index) const
{
    return index.isValid() ? 0 : returnresult.count();
}

//重写设置搜索的model标头
QVariant settingModel::headerData(int section,Qt::Orientation orientation ,int role) const {
    if(role == 0){
        return tr("Settings");
    }
    return QAbstractItemModel::headerData(section,orientation,role);
}

QVariant settingModel::data(const QModelIndex &index, int role) const
{
    if (role == 0) {
        return returnresult.at(index.row());
    }
    return QVariant();
}

//运行对应条目的设置
void settingModel::run(int index)
{
    setting->beginGroup(QString::fromLocal8Bit("control-center"));
    QProcess p;
    p.setProgram(setting->value(commandresult.at(index)).toString());
    p.startDetached(p.program());
    p.waitForFinished(-1);
    setting->endGroup();
}

//按字段解析xml文件，将设置插件的中文提取出来
void settingModel::XmlElement(){
    QFile file(QString::fromLocal8Bit(":/tests/assets/search.xml"));
    if (!file.open(QIODevice::ReadOnly)){
        return;
    }
    QDomDocument doc;
    doc.setContent(&file);
    QDomElement root=doc.documentElement();
    QDomNode node = root.previousSibling();
    node=root.firstChild();
    file.close();

    while(!node.isNull()){
        QDomElement element=node.toElement();
        QString key=element.attribute(QString::fromLocal8Bit("name"));;
        searchresult=searchlist.value(key);
         QDomNodeList list=element.childNodes();
         for(int i=0;i<list.count();++i){
             QDomNode n=list.at(i);
             if(n.nodeName()==QString::fromLocal8Bit("ChinesePlugin")){
                 index=n.toElement().text();
             }
             if(n.nodeName()==QString::fromLocal8Bit("ChineseFunc")){
                 index+=QString::fromLocal8Bit(":")+n.toElement().text();
             searchresult.append(index);
             }
         }
        searchlist.insert(key,searchresult);
        node=node.nextSibling();
    }
}

//匹配初始化
void settingModel::matchstart(const QString &source){
        sourcetext=source;
        returnresult.clear();
        commandresult.clear();
        startmatchTimer->start();
}

//将编辑框的字符串与xml文件解析出的结果正则表达式匹配
void settingModel::matching(){
    sourcetext+=QString::fromLocal8Bit(".*");
    QRegExp rx(sourcetext);
    QMap<QString, QStringList>::const_iterator i;
    for(i=searchlist.constBegin();i!=searchlist.constEnd();++i){
        regmatch=*i;
        settingkey=i.key();
        QList<QString>::Iterator it = regmatch.begin(),itend = regmatch.end();
        int n = 0;
        for (;it != itend; it++,n++){
            if(rx.exactMatch(*it)){
                returnresult.append(*it);
                commandresult.append(settingkey);
            }
        }
    }
    matchesChanged();
}

//编辑栏内容改变，将model重新刷新
void settingModel::matchesChanged()
{
    bool fullReset = false;
    int newCount = returnresult.count();
    int oldCount = lockresult.count();
    if (newCount > oldCount) {
        for (int row = 0; row < oldCount; ++row) {
            if (!(returnresult.at(row) == lockresult.at(row))) {
                fullReset = true;
                break;
            }
        }
        if (!fullReset) {
            beginInsertRows(QModelIndex(), oldCount, newCount-1);
            endInsertRows();
        }
    } else {
        fullReset = true;
    }
    if (fullReset) {
        beginResetModel();
        endResetModel();
    }
    lockresult=returnresult;
}
