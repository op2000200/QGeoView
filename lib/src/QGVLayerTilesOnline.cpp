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

int callback(void *data, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

static int callback2(void* data, int argc, char** argv, char** azColName) {
    std::string* result = static_cast<std::string*>(data);
    
    for (int i = 0; i < argc; i++) {
        // Добавляем данные в строку
        if (argv[i]) {
            *result += azColName[i];
            *result += ": ";
            *result += argv[i];
            *result += "\n"; // Новая строка после каждого столбца
        }
    }
    *result += "\n"; // Новая строка после каждой записи
    return 0;
}

QGVLayerTilesOnline::~QGVLayerTilesOnline()
{
    qDeleteAll(mRequest);
}

void QGVLayerTilesOnline::initDB(QString name)
{
    sqlite3_open(name.toLocal8Bit().data(), &cache);
    sqlite3_enable_load_extension(cache, 1);
    char *err;
    sqlite3_load_extension(cache,QDir::currentPath().append("/fileio.so").toLocal8Bit().data(),0,&err);
    std::cout << err << std::endl;
    sqlite3_exec(cache,"create table if not exists files (name text, data blob);",0,0,&err);
    std::cout << err << std::endl;
    //if (query.exec("SELECT load_extension(\"fileio.c\")"))
    //cache.open();
    //query.exec("create table if not exists files (name text, data blob);");
    //std::cout << cache.tables()[0].toStdString() << std::endl;

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
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            qgvCritical() << "ERROR" << reply->errorString();
        }
        //check if it has that image
        if (true)
        {
        char *err;
            sqlite3_exec(cache,QString("select writefile('" + QString("(%1)tile(%2,%3,%4)%5")
                            .arg(QGVLayerTilesOnline::getName())
                            .arg(tilePos.zoom())
                            .arg(tilePos.pos().x())
                            .arg(tilePos.pos().y())
                            .arg(".png") + "',data) from files where name ='" + QString("(%1)tile(%2,%3,%4)%5")
                            .arg(QGVLayerTilesOnline::getName())
                            .arg(tilePos.zoom())
                            .arg(tilePos.pos().x())
                            .arg(tilePos.pos().y())
                            .arg(".png") + "';")
                            .toLocal8Bit()
                            .data() ,0,0,&err);
            std::cout << err << std::endl;
            QImage img(QString("(%1)tile(%2,%3,%4)%5")
                            .arg(QGVLayerTilesOnline::getName())
                            .arg(tilePos.zoom())
                            .arg(tilePos.pos().x())
                            .arg(tilePos.pos().y())
                            .arg(".png"));
            auto tile = new QGVImage();
            tile->setGeometry(tilePos.toGeoRect());
            tile->loadImage(img);
            tile->setProperty("drawDebug",
                            QString("%1\ntile(%2,%3,%4)")
                                .arg(reply->url().toString())
                                .arg(tilePos.zoom())
                                .arg(tilePos.pos().x())
                                .arg(tilePos.pos().y()));
            removeReply(tilePos);
            onTile(tilePos, tile);
            system("rm *.png");
        }
        else
        {

        }
        return;
    }
    
    const auto rawImage = reply->readAll();
    QImage img;
    img.loadFromData(rawImage);
    img.save(QString("cache/(%1)tile(%2,%3,%4)%5")
                    .arg(QGVLayerTilesOnline::getName())
                    .arg(tilePos.zoom())
                    .arg(tilePos.pos().x())
                    .arg(tilePos.pos().y())
                    .arg(".png"),"PNG");

    //add to db
    char *err;
    std::string res;
    sqlite3_exec(cache,QString("select name from files where name ='" + QString("(%1)tile(%2,%3,%4)%5")
                    .arg(QGVLayerTilesOnline::getName())
                    .arg(tilePos.zoom())
                    .arg(tilePos.pos().x())
                    .arg(tilePos.pos().y())
                    .arg(".png") + "';")
                    .toLocal8Bit()
                    .data() ,callback2,&res,&err);
    //qgvCritical() << "1 " << QString::fromStdString(res) << " " << QString::fromStdString(err) << endl;
    
    //check if empty
    if (true)
    {
        sqlite3_exec(cache,QString("insert into files(name,data) values('" + QString("(%1)tile(%2,%3,%4)%5")
                        .arg(QGVLayerTilesOnline::getName())
                        .arg(tilePos.zoom())
                        .arg(tilePos.pos().x())
                        .arg(tilePos.pos().y())
                        .arg(".png") + "',readfile('" + QString("(%1)tile(%2,%3,%4)%5")
                        .arg(QGVLayerTilesOnline::getName())
                        .arg(tilePos.zoom())
                        .arg(tilePos.pos().x())
                        .arg(tilePos.pos().y())
                        .arg(".png") + "'))")
                        .toLocal8Bit()
                        .data() ,0,0,&err);
        std::cout << err << std::endl;
    }
    else
    {
        //not insert but update
    }
    
    system("rm *.png");

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
    
    
    
    //mine
    /*
    QImage img;
    img.loadFromData(rawImage);
    if(img.save(QString("(%1)tile(%2,%3,%4)%5")
                    .arg(QGVLayerTilesOnline::getName())
                    .arg(tilePos.zoom())
                    .arg(tilePos.pos().x())
                    .arg(tilePos.pos().y())
                    .arg(".png"),"PNG"))
    {
        qgvDebug() << "cached " << QString("(%1)tile(%2,%3,%4)%5")
                                    .arg(QGVLayerTilesOnline::getName())
                                    .arg(tilePos.zoom())
                                    .arg(tilePos.pos().x())
                                    .arg(tilePos.pos().y())
                                    .arg(".png");
    }
    else
    {
        qgvDebug() << "error while caching " << QString("(%1)tile(%2,%3,%4)%5")
                                                    .arg(QGVLayerTilesOnline::getName())
                                                    .arg(tilePos.zoom())
                                                    .arg(tilePos.pos().x())
                                                    .arg(tilePos.pos().y())
                                                    .arg(".png");
    }
    */
    //end of mine
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
