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

#pragma once

#include "QGVLayerTiles.h"

#include <QNetworkReply>

#include <QSql>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <sqlite3.h>

class QGV_LIB_DECL QGVLayerTilesOnline : public QGVLayerTiles
{
    Q_OBJECT

public:
    ~QGVLayerTilesOnline();

protected:
    virtual QString tilePosToUrl(const QGV::GeoTilePos& tilePos) const = 0;
    void initDB(QString name);
    //virtual QSqlDatabase getDB();
    QSqlDatabase *cache;
private:
    void request(const QGV::GeoTilePos& tilePos) override;
    void cancel(const QGV::GeoTilePos& tilePos) override;
    void onReplyFinished(QNetworkReply* reply, const QGV::GeoTilePos& tilePos);
    void removeReply(const QGV::GeoTilePos& tilePos);

    bool isCached();
    bool isOutdated();
    void addToCache();
    //what type ii getting from cache
    void getFromCache();

private:
    QMap<QGV::GeoTilePos, QNetworkReply*> mRequest;
};
