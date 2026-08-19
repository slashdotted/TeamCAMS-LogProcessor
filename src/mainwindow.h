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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openBrowseScriptDialog();
    void openBrowseOutputDirDialog();
    void openAddInputFileDialog();
    void startProcessing();

    void handleLinesCountUpdated(unsigned int);
    void handleStatusUpdated(const QString&);
    void handleCurrentLineUpdated(unsigned int);
    void handleOverrallProgressUpdated(unsigned int);
    void handleCompleted();
    void handleFinished();
    void handleCurrentFileChanged(const QString&);
    void showCritical(const QString&);

    void updateActions();
    void inputFileListContextMenu(QPoint pos);
    void removeInputFile();
    void selectAllInputFiles();
    void deselectAllInputFiles();
    void invertSelection();

    void about();
private:
    struct pimpl;
    std::unique_ptr<pimpl> m_pimpl;
};

#endif // MAINWINDOW_H
