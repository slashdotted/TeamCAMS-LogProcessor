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

#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

#include <QThread>
#include <QString>
#include <QStringList>
#include <QFile>
#include <memory>

class ProcessingThread : public QThread {
    Q_OBJECT
public:
    explicit ProcessingThread(const QString& script, const QStringList& inputfiles, const QString& outputpath, QObject *parent = nullptr);
    ~ProcessingThread() override;
    void run() override;
signals:
    void linesCountUpdated(unsigned int);
    void statusUpdated(const QString&);
    void currentLineUpdated(unsigned int);
    void overrallProgressUpdated(unsigned int);
    void completed();
    void currentFileChanged(const QString&);
    void critical(const QString&);

public slots:
    void initOutputFile(const QString& name);
    void writeLine(const QString& line);
    void closeOutputFile();

private:
    struct pimpl;
    std::unique_ptr<pimpl> m_pimpl;
};

#endif // PROCESSINGTHREAD_H
