/***************************************************************************
 * QGeoView is a Qt / C ++ widget for visualizing geographic data.
 * Copyright (C) 2018-2024 Andrey Yaroshenko.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see https://www.gnu.org/licenses.
 ****************************************************************************/

#include "QGVLayerTilesOnline.h"
#include "Raster/QGVImage.h"
#include <iostream>
#include <QDir>

QGVLayerTilesOnline::~QGVLayerTilesOnline()
{
    qDeleteAll(mRequest);
}

void QGVLayerTilesOnline::initDB(QString name)
{
    cache = new QSqlDatabase;
    cache[0] = QSqlDatabase::addDatabase("QSQLITE");
    //implement database connection names for example:
    //cache[0] = QSqlDatabase::addDatabase("QSQLITE", name);
    cache->setDatabaseName(name);
    cache->open();
    QSqlQuery query;
    query.exec("create table if not exists files(name text, data blob);");
}

QByteArray QGVLayerTilesOnline::getTileFromCache(QString name)
{
    QSqlQuery select;
    if (!select.prepare("select data from files where name = :name"))
        return QByteArray();
    select.bindValue(":name", name);
    if (!select.exec())
        return QByteArray();
    if (!select.next())
        return QByteArray();
    else
    {
        return select.value(0).toByteArray();
    }
}

bool QGVLayerTilesOnline::addTileToCache(QString name, QByteArray data)
{
    QSqlQuery insert;
        if (!insert.prepare("insert into files (name,data) " "values(:name,:data)"))
            return false;
        insert.bindValue(":name", name);
        insert.bindValue(":data", data);
        if (!insert.exec())
            return false;
    return true;
}

void QGVLayerTilesOnline::request(const QGV::GeoTilePos& tilePos)
{
    Q_ASSERT(QGV::getNetworkManager());

    const QUrl url(tilePosToUrl(tilePos));

    QNetworkRequest request(url);
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);

    request.setSslConfiguration(conf);
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (Windows; U; MSIE "
                         "6.0; Windows NT 5.1; SV1; .NET "
                         "CLR 2.0.50727)");
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);

    QNetworkReply* reply = QGV::getNetworkManager()->get(request);

    mRequest[tilePos] = reply;
    connect(reply, &QNetworkReply::finished, reply, [this, reply, tilePos]() { onReplyFinished(reply, tilePos); });


    qgvDebug() << "request" << url;
}

void QGVLayerTilesOnline::cancel(const QGV::GeoTilePos& tilePos)
{
    removeReply(tilePos);
}

void QGVLayerTilesOnline::onReplyFinished(QNetworkReply* reply, const QGV::GeoTilePos& tilePos)
{
    QSqlQuery select;
    QSqlQuery insert;
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qgvCritical() << "ERROR" << reply->errorString();
        }
        QString name = QGVLayerTilesOnline::getName() + QString("tile(%1,%2,%3)")
                        .arg(tilePos.zoom())
                        .arg(tilePos.pos().x())
                        .arg(tilePos.pos().y());
        auto tile = new QGVImage();
        QByteArray data = getTileFromCache(name);
        tile->setGeometry(tilePos.toGeoRect());
        if (!data.isEmpty())
        {
            tile->loadImage(data);
        }
        else
        {
            //no data label not implemented, can be done in default tile constructor
        }
        removeReply(tilePos);
        onTile(tilePos, tile);
        return;
    }

    QString name = QGVLayerTilesOnline::getName() + QString("tile(%1,%2,%3)")
                        .arg(tilePos.zoom())
                        .arg(tilePos.pos().x())
                        .arg(tilePos.pos().y());
    auto tile = new QGVImage();
    QByteArray data = getTileFromCache(name);
    tile->setGeometry(tilePos.toGeoRect());
    if (!data.isEmpty())
    {
        tile->loadImage(data);
    }
    else
    {
        //not found need to load and cache
        const auto rawImage = reply->readAll();
        tile->loadImage(rawImage);
        if (!addTileToCache(name, rawImage))
            qDebug() << "failed to cache" << name;
    }
    removeReply(tilePos);
    onTile(tilePos, tile);
}

void QGVLayerTilesOnline::removeReply(const QGV::GeoTilePos& tilePos)
{
    QNetworkReply* reply = mRequest.value(tilePos, nullptr);
    if (reply == nullptr) {
        return;
    }
    mRequest.remove(tilePos);
    reply->abort();
    reply->close();
    reply->deleteLater();
}
