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
    cache->setDatabaseName(name);
    //std::cout << cache->databaseName().toStdString();
    cache->open();
    QSqlQuery query;
    query.exec("create table if not exists files(name text, data blob);");
    std::cout << cache->tables()[0].toStdString() << std::endl;

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
    QSqlQuery update;
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qgvCritical() << "ERROR" << reply->errorString();
        }
        QString name = QGVLayerTilesOnline::getName() + QString("tile(%1,%2,%3)")
                        .arg(tilePos.zoom())
                        .arg(tilePos.pos().x())
                        .arg(tilePos.pos().y());
        select.exec("select data from files where name='"+ name +"'");
        auto tile = new QGVImage();
        if (!select.next())
        {
            qDebug() << "no data";
            qDebug() << name;
            tile->setGeometry(tilePos.toGeoRect());
            tile->setProperty("drawDebug",
                            QString("NO DATA"));
        }
        qDebug() << "has response, showing";
        qDebug() << name;
        QByteArray data = select.value(0).toByteArray();
        tile->setGeometry(tilePos.toGeoRect());
        tile->loadImage(data);
        tile->setProperty("drawDebug",
                        QString("%1\ntile(%2,%3,%4)")
                            .arg(reply->url().toString())
                            .arg(tilePos.zoom())
                            .arg(tilePos.pos().x())
                            .arg(tilePos.pos().y()));
        removeReply(tilePos);
        onTile(tilePos, tile);
        return;
    }
    
    const auto rawImage = reply->readAll();
    QString name = QGVLayerTilesOnline::getName() + QString("tile(%1,%2,%3)")
                        .arg(tilePos.zoom())
                        .arg(tilePos.pos().x())
                        .arg(tilePos.pos().y());
    //std::cout << name.toStdString() << std::endl;
    select.exec("select data from files where name='"+ name +"'");
    if (!select.next())
    {
        qDebug() << "no responce, adding a row";
        qDebug() << name;
        insert.exec("insert into files(name,data) values('"+ name + "','"+ rawImage.constData() +"')");
    }
    qDebug() << "has response, updating";
    qDebug() << name;
    //insert.exec("insert into files(name,data) values('"+ name + "','"+ rawImage.constData() +"')");
    update.exec(QString("update files set data='").append(rawImage.constData()).append("' where name = '").append(name).append("'"));
    auto tile = new QGVImage();
    tile->setGeometry(tilePos.toGeoRect());
    tile->loadImage(rawImage);
    tile->setProperty("drawDebug",
                    QString("%1\ntile(%2,%3,%4)")
                        .arg(reply->url().toString())
                        .arg(tilePos.zoom())
                        .arg(tilePos.pos().x())
                        .arg(tilePos.pos().y()));
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
