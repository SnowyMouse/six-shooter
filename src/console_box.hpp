// SPDX-License-Identifier: GPL-3.0-only

#include <QTextEdit>

class QProcess;

namespace SixShooter {
    class ConsoleBox : public QTextEdit {
        Q_OBJECT
    public:
        ConsoleBox(QWidget *parent = nullptr);
        void attach_to_process(QProcess *process);
        
    private:
        void on_output();
        void on_standard_output();
        
        void read_everything();
        
        QProcess *process = nullptr;
        
        std::string html;
    };
}
