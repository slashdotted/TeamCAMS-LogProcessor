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

#include "processingthread.h"
#include <QFile>
#include <QJSEngine>
#include <QApplication>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <iostream>
#include <QDir>

struct ProcessingThread::pimpl {
    bool m_fatal_error{false};
    QString m_script_path;
    QStringList m_input_files;
    QString m_output_path;
    QFile* m_current_output{nullptr};
};

static unsigned int lineCount(QFile& f)
{
    unsigned int count{0};
    while(!f.atEnd()) {
        char buf[1024];
        if(f.readLine(buf, sizeof(buf)) != -1) {
            ++count;
        }
    }
    f.seek(0);
    return count;
}

ProcessingThread::ProcessingThread(const QString &script, const QStringList &inputfiles, const QString& outputpath, QObject *parent) :
    QThread{parent}, m_pimpl{std::make_unique<pimpl>()}
{
    m_pimpl->m_script_path = script;
    m_pimpl->m_input_files = inputfiles;
    m_pimpl->m_output_path = outputpath;
}

ProcessingThread::~ProcessingThread()
{
    closeOutputFile();
    qDebug() << "Destroying thread";
}

QStringList parse(const QString& line, const QChar& sep = ';')
{
    bool inquote{false};
    bool escaping{false};
    QStringList result;
    QString current;
    for (const auto &c : line) {
        if (c == '\\') {
            if (!escaping) {
                escaping = true;
                continue;
            }
            else {
                current += "\\\\";
            }

        }
        else if (c == '"') {
            if (!escaping) {
                inquote = !inquote;
            }
            else {
                current += c;
                escaping = false;
            }
        }
        else if (c == sep) {
            if (!escaping) {
                result.push_back(current);
                current.clear();
                continue;
            }
            else {
                current += c;
                escaping = false;
            }
        }
        else if (escaping) {
            current += '\\';
            current += c;
            escaping = false;
        }
        else {
            current += c;
        }
    }
    if (!current.isEmpty()) {
        result.push_back(current.replace('\n',""));
    }
    return result;
}

void ProcessingThread::run()
{
    qDebug() << "Starting processing";
    // Load Script
    unsigned int idx{0};
    for(const auto& currentFile : m_pimpl->m_input_files) {
        if (m_pimpl->m_fatal_error) {
            return;
        }
        ++idx;
        QFile f{currentFile};
        if (!f.open(QIODevice::ReadOnly)) {
            continue;
        }
        QJSEngine m_engine;
        m_engine.installExtensions(QJSEngine::AllExtensions);
        m_engine.globalObject().setProperty("proc", m_engine.newQObject(this));
        QFile script{m_pimpl->m_script_path};
        if (!script.open(QIODevice::ReadOnly)) {
            emit critical(tr("<b>Cannot open script error in script</b><br><br>%1").arg(m_pimpl->m_script_path));
            return;
        }
        QTextStream tstream(&script);
        auto code = tstream.readAll();
        script.close();
        auto result{m_engine.evaluate(code)};
        if (result.isError()) {
            int line = result.property("lineNumber").toInt();
            emit critical(tr("<b>Runtime error loading script</b><br><br>%1 -> %2").arg(code, result.toString()+ " (line "+ QString::number(line)+")"));
            return;
        }
        auto init_result{m_engine.evaluate(QString{"init(\"%1\");"}.arg(QFileInfo{f}.fileName()))};
        if (init_result.isError()) {
            int line = result.property("lineNumber").toInt();
            emit critical(tr("<b>Runtime error calling init in script</b><br><br>%1 -> %2").arg(code, init_result.toString()+ " (line "+ QString::number(line)+")"));
            return;
        }
        unsigned int count{0};
        emit linesCountUpdated(lineCount(f));
        emit currentFileChanged(QFileInfo{f}.fileName());
        while(!f.atEnd()) {
            char buf[16384];
            if(f.readLine(buf, sizeof(buf)) != -1) {
                emit currentLineUpdated(++count);
            }
            auto data{parse(buf)};
            if (data.count() != 14) {
                emit critical(tr("<b>Wrong number of columns in CSV file %1, line %2</b>: expecting 14, found %3; %4").arg(currentFile).arg(idx).arg(data.count()).arg(QString{buf}));
                continue;
            }
            bool ok{false};
            double timestamp{data.at(0).toDouble(&ok)};
            QString tag{data.at(9)};
            QString sender{data.at(13)};
            QString text{data.at(10)};
            QJsonObject obj;
            obj["o2"] = data.at(2).toDouble(&ok);
            obj["pressure"] = data.at(3).toDouble(&ok);
            obj["temperature"] = data.at(4).toDouble(&ok);
            obj["co2"] = data.at(5).toDouble(&ok);
            obj["humidity"] = data.at(6).toDouble(&ok);
            obj["o2tank"] = data.at(7).toDouble(&ok);
            obj["n2tank"] = data.at(8).toDouble(&ok);
            obj["fault"] = data.at(11);
            obj["ticks"] = data.at(1).toDouble(&ok);
            obj["index"] = data.at(12).toDouble(&ok);
            QJsonDocument doc{obj};
            QString status{doc.toJson(QJsonDocument::Compact)};
            QString parseCmd{"parse(%1,\"%2\",\"%3\",\"%4\",%5);"};
            auto parse_result{m_engine.evaluate(parseCmd.arg(timestamp).arg(tag, sender, text.replace("\"", "\\\"")).arg(status))};
            if (parse_result.isError()) {
                int line = result.property("lineNumber").toInt();
                std::cout << parseCmd.arg(timestamp).arg(tag, sender, text.replace("\"", "\\\"")).arg(status).toStdString() << std::endl;
                emit critical(tr("<b>Runtime error calling parse in script</b><br><br>%1 -> %2").arg(code, parse_result.toString()+ " (line "+ QString::number(line)+")"));
                return;
            }
        }
        auto cleanup_result{m_engine.evaluate(QString{"cleanup();"})};
        if (cleanup_result.isError()) {
            int line = result.property("lineNumber").toInt();
            emit critical(tr("<b>Runtime error calling cleanup in script</b><br><br>%1 -> %2").arg(code, cleanup_result.toString()+ " (line "+ QString::number(line)+")"));
            return;
        }
        emit overrallProgressUpdated(idx);
    }
    qDebug() << "Processing completed";
    emit completed();
}

void ProcessingThread::initOutputFile(const QString &name)
{
    if (m_pimpl->m_current_output != nullptr) {
        if (m_pimpl->m_current_output->isOpen()) {
            m_pimpl->m_current_output->close();
        }
        delete m_pimpl->m_current_output;
        m_pimpl->m_current_output = nullptr;
    }
    QDir output_dir{m_pimpl->m_output_path};
    if (!output_dir.exists()) {
        m_pimpl->m_fatal_error = true;
        return;
    }
    m_pimpl->m_current_output = new QFile(output_dir.absoluteFilePath(name));
    if (!m_pimpl->m_current_output->open(QIODevice::WriteOnly)) {
        emit critical(tr("<b>Cannot open output file</b>: %1").arg(name));
        return;
    }
}

void ProcessingThread::writeLine(const QString &line)
{
    if (m_pimpl->m_current_output && m_pimpl->m_current_output->isOpen()) {
        m_pimpl->m_current_output->write((line+'\n').toLocal8Bit());
    }
}

void ProcessingThread::closeOutputFile()
{
    if (m_pimpl->m_current_output != nullptr) {
        if (m_pimpl->m_current_output->isOpen()) {
            m_pimpl->m_current_output->close();
        }
        delete m_pimpl->m_current_output;
        m_pimpl->m_current_output = nullptr;
    }
}
