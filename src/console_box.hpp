// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_CONSOLE_BOX_HPP
#define SIX_SHOOTER_CONSOLE_BOX_HPP

#include <QTextEdit>

class QProcess;

namespace SixShooter {
    class ConsoleBox : public QTextEdit {
        Q_OBJECT
    public:
        enum OutputChannel {
            StandardOutput,
            StandardError
        };

        ConsoleBox(QWidget *parent = nullptr);
        void attach_to_process(QProcess *process, OutputChannel channels);
        void reset_contents();

    private:
        void on_standard_output();
        void on_standard_error();

        QProcess *process = nullptr;

        std::string html;
    };
}

#endif
