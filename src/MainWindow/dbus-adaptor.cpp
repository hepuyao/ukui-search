/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp com.kylin.security.controller.filectrl.xml -i dbus.h -a dbus-adaptor
 *
 * qdbusxml2cpp is Copyright (C) 2016 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "dbus-adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class FilectrlAdaptor
 */

FilectrlAdaptor::FilectrlAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

FilectrlAdaptor::~FilectrlAdaptor()
{
}

bool FilectrlAdaptor::GetApplicationDesktop(const QString &arg)
{
    bool out0;
    QMetaObject::invokeMethod(parent(), "GetApplicationDesktop", Q_RETURN_ARG(bool, out0), Q_ARG(QString, arg));
    return out0;
}

bool FilectrlAdaptor::CheckIfExist(const QString &arg)
{
    bool out0;
    QMetaObject::invokeMethod(parent(), "CheckIfExist", Q_RETURN_ARG(bool, out0), Q_ARG(QString, arg));
    return out0;
}

bool FilectrlAdaptor::GetSearchResult(const QStringList &arg)
{
    bool out0;
    QMetaObject::invokeMethod(parent(), "GetSearchResult", Q_RETURN_ARG(bool, out0), Q_ARG(QStringList, arg));
    return out0;
}
