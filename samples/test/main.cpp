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

#include <QApplication>
#include <QCommandLineParser>

#include "mainwindow.h"

#include <sqlite3.h>


const char* SQL = "CREATE TABLE IF NOT EXISTS foo(a,b,c); INSERT INTO FOO VALUES(1,2,3); INSERT INTO FOO SELECT * FROM FOO;";

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Test");

    MainWindow window;
    window.show();
    return app.exec();
}
