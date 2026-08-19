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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "processingthread.h"
#include <QSettings>
#include <QDebug>
#include <algorithm>

struct MainWindow::pimpl {
    Q_DISABLE_COPY(pimpl)
    Ui::MainWindow *ui;
    QFile* m_inputFile;
    QLabel* m_status_label;
    pimpl(MainWindow* self) : ui{new Ui::MainWindow}
    {
        ui->setupUi(self);
        m_status_label = new QLabel{"Not running", ui->statusBar};
        ui->statusBar->addWidget(m_status_label);
    }
    ~pimpl()
    {
        delete ui;
    }
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_pimpl{std::make_unique<pimpl>(this)}
{
    connect(m_pimpl->ui->actionAdd_input_file, &QAction::triggered, this, &MainWindow::openAddInputFileDialog);
    connect(m_pimpl->ui->actionOpen_processor_script, &QAction::triggered, this, &MainWindow::openBrowseScriptDialog);
    connect(m_pimpl->ui->actionSelect_output_directory, &QAction::triggered, this, &MainWindow::openBrowseOutputDirDialog);
    connect(m_pimpl->ui->actionStart_processing, &QAction::triggered, this, &MainWindow::startProcessing);
    connect(m_pimpl->ui->procScriptEdit, &QLineEdit::textChanged, this, &MainWindow::updateActions);
    connect(m_pimpl->ui->outDirEdit, &QLineEdit::textChanged, this, &MainWindow::updateActions);
    connect(m_pimpl->ui->inputFileList, &QListWidget::customContextMenuRequested, this, &MainWindow::inputFileListContextMenu);
    connect(m_pimpl->ui->actionRemove_input_file, &QAction::triggered, this, &MainWindow::removeInputFile);
    connect(m_pimpl->ui->actionSelect_All, &QAction::triggered, this, &MainWindow::selectAllInputFiles);
    connect(m_pimpl->ui->actionDeselect_All, &QAction::triggered, this, &MainWindow::deselectAllInputFiles);
    connect(m_pimpl->ui->actionInvert_Selection, &QAction::triggered, this, &MainWindow::invertSelection);
    connect(m_pimpl->ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_pimpl->ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    m_pimpl->ui->inputFileList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pimpl->ui->inputFileList->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);

    QSettings settings;
    m_pimpl->ui->procScriptEdit->setText(settings.value("script", "").toString());
    m_pimpl->ui->outDirEdit->setText(settings.value("output", "").toString());
    for (const auto& v : settings.value("input").toStringList()) {
        m_pimpl->ui->inputFileList->addItem(v);
    }

    updateActions();

}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("script", m_pimpl->ui->procScriptEdit->text());
    settings.setValue("output", m_pimpl->ui->outDirEdit->text());
    QStringList inlist;
    for (auto i{0}; i<m_pimpl->ui->inputFileList->count(); ++i) {
        inlist.push_back(m_pimpl->ui->inputFileList->item(i)->text());
    }
    settings.setValue("input", inlist);
}

void MainWindow::openBrowseScriptDialog()
{
    QString path = QDir::homePath();
    if (!m_pimpl->ui->procScriptEdit->text().isEmpty()) {
        QFileInfo fileInfo{m_pimpl->ui->procScriptEdit->text()};
        auto d{fileInfo.dir()};
        if (d.exists()) {
            path = d.absolutePath();
        }
    }
    auto fileName{QFileDialog::getOpenFileName(this,
                  tr("Choose Script File"), path, tr("JS Files (*.js)"))};
    if (!fileName.isEmpty()) {
        m_pimpl->ui->procScriptEdit->setText(fileName);
        updateActions();
    }
}

void MainWindow::openBrowseOutputDirDialog()
{
    auto dirPath{QFileDialog::getExistingDirectory(this,
                 tr("Choose Output Directory"), QDir::homePath())};
    if (!dirPath.isEmpty()) {
        m_pimpl->ui->outDirEdit->setText(dirPath);
        updateActions();
    }
}

void MainWindow::openAddInputFileDialog()
{
    QFileDialog dialog{this};
    dialog.setWindowTitle("Choose Input Files");
    dialog.setNameFilter(tr("CSV Files (*.csv)"));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (m_pimpl->ui->inputFileList->count() > 0) {
        auto path =m_pimpl->ui->inputFileList->item(m_pimpl->ui->inputFileList->count()-1)->text();
        QFileInfo fileInfo{path};
        auto d{fileInfo.dir()};
        if (d.exists()) {
            dialog.setDirectory(d.absolutePath());
        }
    }

    if ( QDialog::Accepted == dialog.exec() ) {
        for (const auto& f : dialog.selectedFiles()) {
            if (!f.isEmpty()) {
                m_pimpl->ui->inputFileList->addItem(f);
            }
        }
    }
    updateActions();
}

void MainWindow::startProcessing()
{
    QStringList inlist;
    for (auto i{0}; i<m_pimpl->ui->inputFileList->count(); ++i) {
        inlist.push_back(m_pimpl->ui->inputFileList->item(i)->text());
    }
    if (!inlist.empty()) {
        m_pimpl->ui->overallProgressBar->setMaximum(inlist.size());
        m_pimpl->ui->overallProgressBar->setValue(0);
        auto procThread{new ProcessingThread{m_pimpl->ui->procScriptEdit->text(), inlist, m_pimpl->ui->outDirEdit->text(), this}};
        connect(procThread, &ProcessingThread::completed, this, &MainWindow::handleCompleted);
        connect(procThread, &ProcessingThread::currentFileChanged, this, &MainWindow::handleCurrentFileChanged);
        connect(procThread, &ProcessingThread::linesCountUpdated, this, &MainWindow::handleLinesCountUpdated);
        connect(procThread, &ProcessingThread::currentLineUpdated, this, &MainWindow::handleCurrentLineUpdated);
        connect(procThread, &ProcessingThread::statusUpdated, this, &MainWindow::handleStatusUpdated);
        connect(procThread, &ProcessingThread::overrallProgressUpdated, this, &MainWindow::handleOverrallProgressUpdated);
        connect(procThread, &ProcessingThread::finished, this, &MainWindow::handleFinished);
        connect(procThread, &ProcessingThread::finished, procThread, &QObject::deleteLater);
        connect(procThread, &ProcessingThread::critical, this, &MainWindow::showCritical, Qt::BlockingQueuedConnection);
        procThread->start();
        m_pimpl->ui->actionStart_processing->setEnabled(false);
        m_pimpl->ui->actionAdd_input_file->setEnabled(false);
        m_pimpl->ui->actionOpen_processor_script->setEnabled(false);
        m_pimpl->ui->actionSelect_output_directory->setEnabled(false);
    }
}

void MainWindow::handleLinesCountUpdated(unsigned int v)
{
    m_pimpl->ui->fileProgressBar->setValue(0);
    m_pimpl->ui->fileProgressBar->setMaximum(v == 0 ? 1 : v);
}

void MainWindow::handleStatusUpdated(const QString &s)
{
    m_pimpl->m_status_label->setText(s);
}

void MainWindow::handleCurrentLineUpdated(unsigned int v)
{
    m_pimpl->ui->fileProgressBar->setValue(v);
}

void MainWindow::handleOverrallProgressUpdated(unsigned int v)
{
    m_pimpl->ui->overallProgressBar->setValue(v);
}

void MainWindow::handleCompleted()
{
    m_pimpl->ui->overallProgressBar->setValue(0);
    m_pimpl->ui->fileProgressBar->setValue(0);
    m_pimpl->m_status_label->setText(tr("Not running"));
    m_pimpl->ui->actionAdd_input_file->setEnabled(true);
    m_pimpl->ui->actionOpen_processor_script->setEnabled(true);
    m_pimpl->ui->actionSelect_output_directory->setEnabled(true);
    updateActions();
}

void MainWindow::handleFinished()
{
    m_pimpl->ui->actionAdd_input_file->setEnabled(true);
    m_pimpl->ui->actionOpen_processor_script->setEnabled(true);
    m_pimpl->ui->actionSelect_output_directory->setEnabled(true);
    updateActions();
}

void MainWindow::handleCurrentFileChanged(const QString &v)
{
    m_pimpl->m_status_label->setText(tr("Processing file %1").arg(v));
}

void MainWindow::showCritical(const QString &msg)
{
    QMessageBox::critical(static_cast<MainWindow*>(parent()), tr("Critical"), msg, QMessageBox::Ok);
}

void MainWindow::updateActions()
{
    if (m_pimpl->ui->procScriptEdit->text().trimmed().isEmpty() || m_pimpl->ui->inputFileList->count() == 0
            || m_pimpl->ui->outDirEdit->text().trimmed().isEmpty()) {
        m_pimpl->ui->actionStart_processing->setEnabled(false);
    }
    else {
        m_pimpl->ui->actionStart_processing->setEnabled(true);
    }
}

void MainWindow::inputFileListContextMenu(QPoint pos)
{
    QModelIndex index=m_pimpl->ui->inputFileList->indexAt(pos);
    if (index.isValid()) {
        auto menu{new QMenu{this}};
        menu->addAction(m_pimpl->ui->actionSelect_All);
        if (!m_pimpl->ui->inputFileList->selectionModel()->selectedRows().empty()) {
            menu->addAction(m_pimpl->ui->actionDeselect_All);
            menu->addAction(m_pimpl->ui->actionInvert_Selection);
            menu->addSeparator();
            menu->addAction(m_pimpl->ui->actionRemove_input_file);
        }
        menu->popup(m_pimpl->ui->inputFileList->viewport()->mapToGlobal(pos));
    }
}

void MainWindow::removeInputFile()
{
    if (m_pimpl->ui->inputFileList->selectionModel()->selectedRows().empty()) return;
    auto selection {m_pimpl->ui->inputFileList->selectionModel()->selectedRows()};
    std::sort(selection.begin(), selection.end(), std::less<QModelIndex>());
    std::reverse(selection.begin(), selection.end());
    for (QModelIndex& e : selection) {
        m_pimpl->ui->inputFileList->model()->removeRow(e.row());
    }
    updateActions();
}

void MainWindow::selectAllInputFiles()
{
    m_pimpl->ui->inputFileList->selectAll();
}

void MainWindow::deselectAllInputFiles()
{
    m_pimpl->ui->inputFileList->clearSelection();
}

void MainWindow::invertSelection()
{
    QModelIndex rootIndex{m_pimpl->ui->inputFileList->rootIndex()};
    QModelIndex first{m_pimpl->ui->inputFileList->model()->index(0, 0)};
    int numOfItems{m_pimpl->ui->inputFileList->model()->rowCount(rootIndex)};
    QModelIndex last{m_pimpl->ui->inputFileList->model()->index(numOfItems - 1, 0)};
    QItemSelection selection{first, last};
    m_pimpl->ui->inputFileList->selectionModel()->select(selection, QItemSelectionModel::Toggle);
}

void MainWindow::about()
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::NoIcon);
    msgBox->setWindowTitle(tr("About TeamCAMS"));
    msgBox->setText(tr("<h3>TeamCAMS Log Processor 2026.08</h3>"

                       "<p>"
                       "Copyright &copy; 2015-2026 "
                       "<b>Amos Brocco</b> / University of Fribourg (Switzerland)."
                       "</p>"

                       "<p>"
                       "This software is free software released under the terms of the "
                       "<b>GNU General Public License version 3 (GPLv3)</b>. "
                       "You are free to use, study, modify and redistribute it under the "
                       "conditions of that license."
                       "</p>"

                       "<p>"
                       "<b>Disclaimer:</b> This program is provided <i>AS IS</i>, without "
                       "any express or implied warranty, including but not limited to the "
                       "warranties of merchantability, fitness for a particular purpose, "
                       "and non-infringement."
                       "</p>"));
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->open();
}
