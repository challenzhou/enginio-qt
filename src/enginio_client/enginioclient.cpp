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

#include "enginioclient_p.h"
#include "enginiojsonobjectfactory.h"
#include "enginioreply.h"
#include "enginiomodel.h"
#include "enginioidentity.h"

#include <QDebug>
#include <QNetworkReply>
#include <QSslError>

/*!
 * \module enginio-client
 * \title Enginio Client Interface
 * 
 * This interface provides access to the Enginio service through a set of C++ classes
 * based on Qt.
 */

/*!
 * \class EnginioClient
 * \inmodule enginio-client
 * \brief Enginio client class
 * Used for handling API keys, sessions and authorization.
 */

/*!
 * \fn EnginioClient::sessionAuthenticated() const
 * \brief Emitted when user logs in.
 */

/*!
 * \fn EnginioClient::sessionTerminated() const
 * \brief Emitted when user logs out.
 */

int FactoryUnit::nextId = 0;

EnginioClientPrivate::EnginioClientPrivate(EnginioClient *client) :
    q_ptr(client),
    _identity(),
    m_apiUrl(QStringLiteral("https://api.engin.io")),
    m_networkManager(),
    m_deleteNetworkManager(true)
{
    addFactory(new EnginioJsonObjectFactory());

    _request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/json"));
    qRegisterMetaType<EnginioClient*>();
    qRegisterMetaType<EnginioModel*>();
    qRegisterMetaType<EnginioReply*>();
    qRegisterMetaType<EnginioIdentity*>();
    qRegisterMetaType<EnginioAuthentication*>();
}

EnginioClientPrivate::~EnginioClientPrivate()
{
    if (m_deleteNetworkManager)
        delete m_networkManager;

    while (m_factories.size() > 0) {
        FactoryUnit *unit = m_factories.takeFirst();
        delete unit->factory;
        delete unit;
    }
}

int EnginioClientPrivate::addFactory(EnginioAbstractObjectFactory *factory)
{
    FactoryUnit *unit = new FactoryUnit;
    unit->factory = factory;
    unit->id = FactoryUnit::nextId++;
    m_factories.prepend(unit);
    return unit->id;
}

void EnginioClientPrivate::removeFactory(int factoryId)
{
    for (int i = 0; i < m_factories.size(); ++i) {
        if (m_factories.at(i)->id == factoryId) {
            FactoryUnit *unit = m_factories.takeAt(i);
            delete unit->factory;
            delete unit;
            return;
        }
    }
}

/*!
 * Create a new client object. \a backendId and \a backendSecret define which
 * Enginio backend will be used with this client. Both can be found from the
 * Enginio dashboard. \a parent is optional.
 */
EnginioClient::EnginioClient(const QString &backendId,
                             const QString &backendSecret,
                             QObject *parent)
    : QObject(parent)
    , d_ptr(new EnginioClientPrivate(this))
{
    qDebug() << this << "created. Backend ID:" << backendId;

    setBackendId(backendId);
    setBackendSecret(backendSecret);
}

EnginioClient::EnginioClient(QObject *parent)
    : QObject(parent)
    , d_ptr(new EnginioClientPrivate(this))
{
}

/*!
 * Constructor used in inheriting classes.
 */
EnginioClient::EnginioClient(const QString &backendId,
                             const QString &backendSecret,
                             EnginioClientPrivate &dd,
                             QObject *parent) :
    QObject(parent),
    d_ptr(&dd)
{
    qDebug() << this << "created. Backend ID:" << backendId;

    setBackendId(backendId);
    setBackendSecret(backendSecret);
}

/*!
 * Destructor.
 */
EnginioClient::~EnginioClient()
{
    qDebug() << this << "deleted";
    delete d_ptr;
}

/*!
 * Get the Enginio backend ID.
 */
QString EnginioClient::backendId() const
{
    Q_D(const EnginioClient);
    return d->m_backendId;
}

/*!
 * Change Enginio backend ID to \a backendId.
 */
void EnginioClient::setBackendId(const QString &backendId)
{
    Q_D(EnginioClient);
    if (d->m_backendId != backendId) {
        d->m_backendId = backendId;
        d->_request.setRawHeader("Enginio-Backend-Id", d->m_backendId.toLatin1());
        emit backendIdChanged(backendId);
        if (isInitialized())
            emit clientInitialized();
    }
}

/*!
 * Get the Enginio backend secret.
 */
QString EnginioClient::backendSecret() const
{
    Q_D(const EnginioClient);
    return d->m_backendSecret;
}

/*!
 * Change Enginio backend secret to \a backendSecret.
 */
void EnginioClient::setBackendSecret(const QString &backendSecret)
{
    Q_D(EnginioClient);
    if (d->m_backendSecret != backendSecret) {
        d->m_backendSecret = backendSecret;
        d->_request.setRawHeader("Enginio-Backend-Secret", d->m_backendSecret.toLatin1());
        emit backendSecretChanged(backendSecret);
        if (isInitialized())
            emit clientInitialized();
    }
}

/*!
 * \qproperty EnginioClient::apiUrl
 * \brief Enginio backend URL.
 *
 * Usually it is not needed to change the default URL.
 */
QUrl EnginioClient::apiUrl() const
{
    Q_D(const EnginioClient);
    return d->m_apiUrl;
}

void EnginioClient::setApiUrl(const QUrl &apiUrl)
{
    Q_D(EnginioClient);
    if (d->m_apiUrl != apiUrl) {
        d->m_apiUrl = apiUrl;
        emit apiUrlChanged();
    }
}

/*!
 * Get the QNetworkAccessManager used by the Enginio library. Note that it
 * will be deleted with the client object.
 */
QNetworkAccessManager * EnginioClient::networkManager()
{
    Q_D(EnginioClient);
    if (d->m_networkManager.isNull()) {
        d->m_networkManager = new QNetworkAccessManager(this);
        QObject::connect(d->m_networkManager.data(), &QNetworkAccessManager::finished, EnginioClientPrivate::ReplyFinishedFunctor{d});
        d->m_deleteNetworkManager = true;

        // Ignore SSL errors when staging backend is used.
        if (apiUrl() == QStringLiteral("https://api.staging.engin.io")) {
            qWarning() << "SSL errors will be ignored";
            connect(d->m_networkManager,
                    SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)),
                    this,
                    SLOT(ignoreSslErrors(QNetworkReply*, const QList<QSslError> &)));
        }
    }
    return d->m_networkManager;
}

/*!
 * Set Enginio library to use existing QNetworkAccessManager instance,
 * \a networkManager, for all network traffic.
 */
void EnginioClient::setNetworkManager(QNetworkAccessManager *networkManager)
{
    Q_D(EnginioClient);
    if (d->m_deleteNetworkManager && !d->m_networkManager.isNull())
        delete d->m_networkManager;

    d->m_networkManager = networkManager;
    // TODO it will crash soon, we need to disconnect in ~EnginioClient
    QObject::connect(d->m_networkManager.data(), &QNetworkAccessManager::finished, EnginioClientPrivate::ReplyFinishedFunctor{d});
    d->m_deleteNetworkManager = false;
}

/*!
 * \property EnginioClient::sessionToken
 * \brief The token of currently authenticated session.
 *
 * Returns empty string if there is no authenticated session.
 *
 * \sa EnginioIdentityAuthOperation::attachToSessionWithToken()
 */
QByteArray EnginioClient::sessionToken() const
{
    Q_D(const EnginioClient);
    return d->sessionToken();
}

void EnginioClient::setSessionToken(const QByteArray &sessionToken)
{
    Q_D(EnginioClient);
    if (d->sessionToken() != sessionToken) {
        d->setSessionToken(sessionToken);
    }
}

bool EnginioClient::isInitialized() const
{
    Q_D(const EnginioClient);
    return d->isInitialized();
}

/*!
 * Register object factory, \a factory, for custom object classes. Only used when
 * you implement object class(es) as subclass of EnginioAbstractObject. If
 * there are no factories that can create objects of type 'x', internal
 * EnginioJsonObjectFactory is used to create EnginioJsonObject instances where,
 * \c objectType property is set to 'x'.
 *
 * Calling this function will take the ownership of the factory object.
 *
 * Returns unique ID for registered factory which can be used to unregister the
 * factory.
 */
int EnginioClient::registerObjectFactory(EnginioAbstractObjectFactory *factory)
{
    Q_D(EnginioClient);
    return d->addFactory(factory);
}

/*!
 * Unregister custom object factory. \a factoryId is the ID received from
 * EnginioClient::registerObjectFactory()
 */
void EnginioClient::unregisterObjectFactory(int factoryId)
{
    Q_D(EnginioClient);
    d->removeFactory(factoryId);
}

/*!
 * Create new object of specified \a type and optionally with \a id.
 * Note that types of user-defined objects have "objects." prefix.
 */
EnginioAbstractObject * EnginioClient::createObject(const QString &type,
                                                    const QString &id) const
{
    Q_D(const EnginioClient);
    EnginioAbstractObject *obj = 0;

    for (int i = 0; i < d->m_factories.size(); ++i) {
        obj = d->m_factories.at(i)->factory->createObjectForType(type, id);
        if (obj)
            break;
    }

    return obj;
}

void EnginioClient::ignoreSslErrors(QNetworkReply* reply,
                                    const QList<QSslError> &errors)
{
    QList<QSslError>::ConstIterator i = errors.constBegin();
    while (i != errors.constEnd()) {
        qWarning() << "Ignoring SSL error:" << i->errorString();
        i++;
    }
    reply->ignoreSslErrors(errors);
}

EnginioReply* EnginioClient::query(const QJsonObject &query, const Area area)
{
    Q_D(EnginioClient);

    QNetworkReply *nreply = d->query(query, area);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);
    return ereply;
}

EnginioReply* EnginioClient::create(const QJsonObject &object, const Area area)
{
    Q_D(EnginioClient);

    if (object.empty())
        return 0;

    QNetworkReply *nreply = d->create(object, area);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

EnginioReply* EnginioClient::update(const QJsonObject &object, const Area area)
{
    Q_D(EnginioClient);

    if (object.empty())
        return 0;

    QNetworkReply *nreply = d->update(object, area);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

EnginioReply* EnginioClient::remove(const QJsonObject &object, const Area area)
{
    Q_D(EnginioClient);

    if (object.empty())
        return 0;

    QNetworkReply *nreply = d->remove(object, area);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}

EnginioIdentity *EnginioClient::identity() const
{
    Q_D(const EnginioClient);
    return d->identity();
}

void EnginioClient::setIdentity(EnginioIdentity *identity)
{
    Q_D(EnginioClient);
    if (d->_identity == identity)
        return;
    d->setIdentity(identity);
}

/*!
 * \brief EnginioClient::uploadFile uploads a file
 * \param associatedObject an existing object on the server
 * \param file the file to upload
 * \return
 *
 * Each uploaded file needs to be associated with an object in the database.
 * If there is no association, the file will eventually get deleted.
 * When an object which had a file associated gets deleted, the file will
 * automatically be deleted as well.
 */
EnginioReply* EnginioClient::uploadFile(const QJsonObject &associatedObject, const QUrl &file)
{
    Q_D(EnginioClient);

    if (associatedObject[QStringLiteral("object")].toObject()[QStringLiteral("objectType")].toString().isEmpty())
        return 0;

    QNetworkReply *nreply = d->uploadFile(associatedObject, file);
    EnginioReply *ereply = new EnginioReply(d, nreply);
    nreply->setParent(ereply);

    return ereply;
}
