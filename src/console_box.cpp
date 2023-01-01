// SPDX-License-Identifier: GPL-3.0-only

#include <QFont>
#include <QStyle>
#include <QFontDatabase>
#include <QApplication>
#include <QProcess>
#include <QScrollBar>
#include <QTextDocument>
#include <optional>

#include "console_box.hpp"

#define TEXT_COLOR "#EEE"

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

    void ConsoleBox::reset_contents() {
        this->html = {};
        this->clear();
    }

    void ConsoleBox::attach_to_process(QProcess *process, OutputChannel channel) {
        if(channel == OutputChannel::StandardError) {
            connect(process, &QProcess::readyReadStandardError, this, &ConsoleBox::on_standard_error);
        }
        else if(channel == OutputChannel::StandardOutput) {
            connect(process, &QProcess::readyReadStandardOutput, this, &ConsoleBox::on_standard_output);
        }

        this->process = process;
    }

    static void vt100(QString &string) {
        // Close these
        string.replace("\x1B[m", "</span>");

        // Look for more
        while(true) {
            auto next_index = string.indexOf("\x1B[");
            if(next_index == -1) {
                break;
            }

            auto cursor = next_index + 2;
            auto next_char_maybe = [&cursor, &string]() -> std::optional<QChar> {
                if(cursor == string.size()) {
                    return std::nullopt;
                }
                else {
                    return string.at(cursor++);
                }
            };

            QString val;

            QString css;
            QString color = TEXT_COLOR;

            auto get_next_value = [&next_char_maybe, &cursor](bool &done) -> std::optional<QString> {
                QString s;

                while(true) {
                    auto next_char = next_char_maybe();
                    if(!next_char.has_value()) {
                        return std::nullopt;
                    }

                    if(*next_char == ';' || *next_char == 'm') {
                        done = *next_char == 'm';
                        break;
                    }

                    s += *next_char;
                }

                return s;
            };

            QStringList all_vals;

            while(true) {
                bool done;
                auto next_value = get_next_value(done);
                if(!next_value.has_value()) {
                    std::printf("VT100 fail: Unexpected end?\n");
                    return;
                }
                all_vals << *next_value;
                if(done) {
                    break;
                }
            }

            int index = 0;
            while(index < all_vals.size()) {
                auto next_value = all_vals[index++];

                auto set_color_based_on_code = [&color](int code) {
                    switch(code) {
                        case 0:
                            color = "#000000";
                            return;
                        case 1:
                            color = "#FF0000";
                            return;
                        case 2:
                            color = "#00FF00";
                            return;
                        case 3:
                            color = "#FFFF00";
                            return;
                        case 4:
                            color = "#0000FF";
                            return;
                        case 5:
                            color = "#FF00FF";
                            return;
                        case 6:
                            color = "#FFFF00";
                            return;
                        case 7:
                            color = "#FFFFFF";
                            return;
                        case 8:
                            color = "#3F3F3F";
                            return;
                        case 9:
                            color = "#7F0000";
                            return;
                        case 10:
                            color = "#007F00";
                            return;
                        case 11:
                            color = "#7F7F00";
                            return;
                        case 12:
                            color = "#00007F";
                            return;
                        case 13:
                            color = "#7F007F";
                            return;
                        case 14:
                            color = "#7F7F00";
                            return;
                        case 15:
                            color = "#7F7F7F";
                            return;
                        default:
                            color = TEXT_COLOR;
                            break;
                    }
                };

                bool ok;
                int n = (next_value).toInt(&ok, 10);
                if(!ok) {
                    std::printf("VT100 fail: Failed to parse %s into a number\n", next_value.toLatin1().data());
                    break;
                }

                switch(n) {
                    case 0:
                        css.clear();
                        color = TEXT_COLOR;
                        break;
                    case 1:
                        css += "font-weight:bold;";
                        break;
                    case 2:
                        css += "opacity:0.75;";
                        break;
                    case 4:
                        css += "text-style:underline;";
                        break;
                    case 8:
                        css += "opacity:0.0;";
                        break;
                    case 30:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                    case 35:
                    case 36:
                    case 37:
                        set_color_based_on_code(n - 30);
                        break;
                    case 38: {
                        auto type = all_vals[index++];

                        if(type == "5") {
                            auto code = all_vals[index++];
                            int n = code.toInt(&ok, 10);
                            if(!ok) {
                                std::printf("VT100 fail: Invalid color code?\n");
                                break;
                            }
                            set_color_based_on_code(n);
                        }
                        else {
                            std::printf("VT100 fail: Unknown color format %s?\n", type.toLatin1().data());
                            break;
                        }
                    }
                }
            }

            QString s = "</span><span style=\"color:" + color + ";" + css + "\">";
            string.replace(next_index, cursor - next_index, s);
        }
    }

    static void clean_string(QString &string) {
        string.replace("\n", "<br/>");
        string.replace("\r", "");
        string.replace(" ", "&nbsp;");

        // Handle VT100 text colors
        vt100(string);
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

        this->html += (QString("<span style=\"color: " TEXT_COLOR "\">") + stderr_data + "</span>").toStdString();

        this->setHtml(this->html.c_str());
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
    }
}
