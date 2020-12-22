// SPDX-License-Identifier: GPL-3.0-only

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
        
    private:
        void on_standard_output();
        void on_standard_error();
        
        QProcess *process = nullptr;
        
        std::string html;
    };
}
