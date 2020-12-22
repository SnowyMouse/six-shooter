// SPDX-License-Identifier: GPL-3.0-only

#include <QFont>
#include <QStyle>
#include <QFontDatabase>
#include <QProcess>
#include <QScrollBar>

#include "console_box.hpp"

namespace SixShooter {
    ConsoleBox::ConsoleBox(QWidget *parent) : QTextEdit(parent) {
        this->setReadOnly(true);
        auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        
        this->setMinimumWidth(QFontMetrics(font).horizontalAdvance('m') * 80);
        this->setMinimumHeight(QFontMetrics(font).ascent() * 24);
        this->setFont(font);
        
        this->setStyleSheet("QTextEdit { background-color: #000; color: #EEE; }");
    }
    
    void ConsoleBox::attach_to_process(QProcess *process) {
        connect(process, &QProcess::readyReadStandardError, this, &ConsoleBox::on_output);
        connect(process, &QProcess::readyReadStandardOutput, this, &ConsoleBox::on_output);
        this->process = process;
    }
    
    void ConsoleBox::on_output() {
        auto stdout_data = QString(this->process->readAllStandardOutput());
        stdout_data.replace("\n", "<br/>");
        stdout_data.replace("\r", "");
        stdout_data.replace(" ", "&nbsp;");
        this->html += (QString("<span style=\"color: #EEE\">") + stdout_data + "</span>").toStdString();
        
        auto stderr_data = QString(this->process->readAllStandardError());
        stderr_data.replace("\n", "<br/>");
        stderr_data.replace("\r", "");
        stderr_data.replace(" ", "&nbsp;");
        this->html += (QString("<span style=\"color: red\">") + stderr_data + "</span>").toStdString();
        
        this->setHtml(this->html.c_str());
        
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
    }
}
