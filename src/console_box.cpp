// SPDX-License-Identifier: GPL-3.0-only

#include <QFont>
#include <QStyle>
#include <QFontDatabase>

#include "console_box.hpp"

namespace SixShooter {
    ConsoleBox::ConsoleBox(QWidget *parent) : QTextEdit(parent) {
        this->setReadOnly(true);
        auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        this->setMinimumWidth(QFontMetrics(font).horizontalAdvance('m') * 80);
        this->setFont(font);
        
        this->setStyleSheet("QTextEdit { background-color: #000; color: #EEE; }");
    }
}
