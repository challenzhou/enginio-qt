/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://qt.digia.com/contact-us
**
** This file is part of the Enginio Qt Client Library.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

#ifndef ENGINIOMODEL_H
#define ENGINIOMODEL_H

#include <QAbstractListModel>
#include <QtCore/qjsonobject.h>
#include <QtCore/qscopedpointer.h>

#include "enginioclient.h"

class EnginioModelPrivate;
class ENGINIOCLIENT_EXPORT EnginioModel : public QAbstractListModel
{
    Q_OBJECT
public:
    // TODO: ctor with client
    explicit EnginioModel(QObject *parent = 0);
    ~EnginioModel();

    Q_PROPERTY(EnginioClient *enginio READ enginio WRITE setEnginio NOTIFY enginioChanged REVISION 1)
    Q_PROPERTY(QJsonObject query READ query WRITE setQuery NOTIFY queryChanged REVISION 1)
    Q_PROPERTY(int operation READ operation WRITE setOperation NOTIFY operationChanged REVISION 1)

    // TODO: that is a pretty silly name
    EnginioClient *enginio() const;
    void setEnginio(const EnginioClient *enginio);

    QJsonObject query();
    void setQuery(const QJsonObject &query);

    EnginioClient::Operation operation() const;
    void setOperation(const int opertaion);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    virtual void fetchMore(const QModelIndex &parent) Q_DECL_OVERRIDE;
    virtual bool canFetchMore(const QModelIndex &parent) const Q_DECL_OVERRIDE;

    Q_INVOKABLE Q_REVISION(1) void append(const QJsonObject &value);
    Q_INVOKABLE Q_REVISION(1) void remove(int row);
    Q_INVOKABLE Q_REVISION(1) void setProperty(int row, const QString &role, const QVariant &value);

protected:
    virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

signals:
    Q_REVISION(1) void operationChanged(const EnginioClient::Operation operation);
    Q_REVISION(1) void queryChanged(const QJsonObject query);
    Q_REVISION(1) void enginioChanged(EnginioClient *enginio);

private:
    Q_DISABLE_COPY(EnginioModel)
    QScopedPointer<EnginioModelPrivate> d;
    friend class EnginioModelPrivate;
};

#endif // ENGINIOMODEL_H
