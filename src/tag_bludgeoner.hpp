// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_TAG_BLUDGEONER_HPP
#define SIX_SHOOTER_TAG_BLUDGEONER_HPP

#include <QDialog>
#include <string>
#include <vector>
#include <filesystem>

class QComboBox;
class QProcess;

namespace SixShooter {
    class MainWindow;
    class ConsoleBox;
    
    class TagBludgeoner : public QDialog {
        Q_OBJECT
        friend class MainWindow;
        
    private:
        TagBludgeoner(const MainWindow *main_window);
        const MainWindow *main_window;
        
        QComboBox *tags;
        QComboBox *level;
        QProcess *process = nullptr;
        ConsoleBox *console_box_stdout;
        ConsoleBox *console_box_stderr;
        
        void bludgeon_tags();
        ~TagBludgeoner();
    };
}

#endif
