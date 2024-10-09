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

#include "mainwindow.h"

#include <QCheckBox>
#include <QTimer>
#include <QVBoxLayout>

#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>

#include <QGeoView/QGVLayerBing.h>
#include <QGeoView/QGVLayerGoogle.h>
#include <QGeoView/QGVLayerOSM.h>

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRadioButton>

MainWindow::MainWindow()
{
    setWindowTitle("QGeoView Samples - debug output");

    setCentralWidget(new QWidget());
    centralWidget()->setLayout(new QVBoxLayout());

    //Helpers::setupCachedNetworkAccessManager(this);
    auto manager = new QNetworkAccessManager(this);
    QGV::setNetworkManager(manager);

    mMap = new QGVMap(this);
    centralWidget()->layout()->addWidget(mMap);

    // Widgets
    mMap->addWidget(new QGVWidgetZoom());

    // Background layer
    auto osmLayer = new QGVLayerOSM();
    mMap->addItem(osmLayer);

    // Options list
    centralWidget()->layout()->addWidget(createOptionsList());


    QTimer::singleShot(100, this, [this]() { mMap->cameraTo(QGVCameraActions(mMap).scaleTo(QGV::GeoRect(QGV::GeoPos(67.204032,149.334276),QGV::GeoPos(-32.842674,482.833131)))); });
}

MainWindow::~MainWindow()
{
}

QGroupBox* MainWindow::createOptionsList()
{
    const QList<QPair<QString, QGVLayer*>> layers = {
        { "GOOGLE_SCHEMA", new QGVLayerGoogle(QGV::TilesType::Schema) },
        { "BING_SCHEMA", new QGVLayerBing(QGV::TilesType::Schema) },
        { "OSM", new QGVLayerOSM() },
    };

    QGroupBox* groupBox = new QGroupBox(tr("Options"));
    groupBox->setLayout(new QVBoxLayout);

    QButtonGroup* group = new QButtonGroup(this);
    QRadioButton* radioButton;

    for (auto pair : layers) {
        auto name = pair.first;
        auto layer = pair.second;

        layer->hide();
        mMap->addItem(layer);

        radioButton = new QRadioButton(name);
        group->addButton(radioButton);

        connect(radioButton, &QRadioButton::clicked, this, [layer, layers](const bool checked) {
            if (!checked)
                return;

            for (auto pair : layers) {
                auto layer = pair.second;
                layer->hide();
            }
            layer->show();
        });

        if (name == "CUSTOM_OSM") {
            auto* widget = new QWidget();
            widget->setLayout(new QHBoxLayout);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            widget->layout()->setContentsMargins(QMargins{ 0, 0, 0, 0 });
#else
            widget->layout()->setMargin(0);
#endif
        } else {
            groupBox->layout()->addWidget(radioButton);
        }
    }

    // Last map will be default
    radioButton->click();

    return groupBox;
}
