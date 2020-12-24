// SPDX-License-Identifier: GPL-3.0-only

#include <QFont>
#include <QStyle>
#include <QFontDatabase>
#include <QApplication>
#include <QProcess>
#include <QScrollBar>
#include <QTextDocument>

#include "console_box.hpp"

#define TEXT_COLOR "#EEE"
#define ERROR_COLOR "#F77"

namespace SixShooter {
    ConsoleBox::ConsoleBox(QWidget *parent) : QTextEdit(parent) {
        this->setReadOnly(true);
        auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        
        auto margins = this->contentsMargins();
        auto document_margin = this->document()->documentMargin();
        
        this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        this->setFont(font);
        this->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);
        auto font_metrics = QFontMetrics(font);
        auto *style = qApp->style();
        this->setFixedWidth(font_metrics.horizontalAdvance("It seems that this whole sentence is exactly eighty characters in string length.") + style->pixelMetric(QStyle::PM_ScrollBarExtent) + this->frameWidth() * 2 + margins.left() + margins.right() + document_margin * 2);
        this->setMinimumHeight(font_metrics.ascent() * 24 + this->frameWidth() * 2 + margins.top() + margins.bottom() + document_margin * 2);
        
        this->setStyleSheet("QTextEdit { background-color: #000; color: " TEXT_COLOR ";}");
    }
    
    void ConsoleBox::attach_to_process(QProcess *process, OutputChannel channel) {
        if(channel == OutputChannel::StandardError) {
            connect(process, &QProcess::readyReadStandardError, this, &ConsoleBox::on_standard_error);
        }
        else if(channel == OutputChannel::StandardOutput) {
            connect(process, &QProcess::readyReadStandardOutput, this, &ConsoleBox::on_standard_output);
        }
        
        this->process = process;
        this->html = {};
        this->clear();
    }
    
    static void clean_string(QString &string) {
        string.replace("\n", "<br/>");
        string.replace("\r", "");
        string.replace(" ", "&nbsp;");
    }
    
    void ConsoleBox::on_standard_output() {
        auto stdout_data = QString(this->process->readAllStandardOutput());
        clean_string(stdout_data);
        
        this->html += (QString("<span style=\"color: " TEXT_COLOR "\">") + stdout_data + "</span>").toStdString();
        
        this->setHtml(this->html.c_str());
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
    }
    
    void ConsoleBox::on_standard_error() {
        auto stderr_data = QString(this->process->readAllStandardError());
        clean_string(stderr_data);
        
        this->html += (QString("<span style=\"color: " ERROR_COLOR "\">") + stderr_data + "</span>").toStdString();
        
        this->setHtml(this->html.c_str());
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
    }
}
