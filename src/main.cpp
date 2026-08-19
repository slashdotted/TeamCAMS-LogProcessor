// TeamCAMS - reborn Cabin Air Management System
// Copyright (C) 2015-2026  Amos Brocco,
//                          Cognitive Ergonomics and Work Psychology Team,
//                          Psychology Department of Fribourg University,
//                          Switzerland / Department of Innovative Technologies
//                          University of Applied Sciences and Arts of Southern
//                          Switzerland, Contact: amos.brocco@supsi.ch
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <QApplication>
#include <QSettings>
#include <QStyleFactory>
#include <QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication a(argc, argv);
    QGuiApplication::setOrganizationName("University of Fribourg");
    QGuiApplication::setOrganizationDomain("unifr.ch");
    QGuiApplication::setApplicationDisplayName("TeamCAMS Log Processor");

    // Configure language settings
    QTranslator translator;
    QSettings settings;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    if (!settings.contains("language")) {
        settings.setValue("language", QLocale::system().name().left(2));
    }
    QString preferredLanguage = settings.value("language").toString();
    const QString baseName = "teamcamslogprocessor_" + preferredLanguage;
    if (translator.load(":/i18n/" + baseName)) {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
