// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_CONSOLE_DIALOG_HPP
#define SIX_SHOOTER_CONSOLE_DIALOG_HPP

#include <QDialog>

class QProcess;

namespace SixShooter {
    class ConsoleBox;

    class ConsoleDialog : public QDialog {
        Q_OBJECT

    public:
        virtual ~ConsoleDialog() = 0;

    protected:
        ConsoleDialog();
        void attach_to_process(QProcess *process);
        void reset_contents();
        QWidget *get_console_widget();

    private:
        ConsoleBox *stderr_box;
        ConsoleBox *stdout_box;
        QWidget *console_widget;
    };
}

#endif
