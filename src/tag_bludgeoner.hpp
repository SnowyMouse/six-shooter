// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_TAG_BLUDGEONER_HPP
#define SIX_SHOOTER_TAG_BLUDGEONER_HPP

#include <QDialog>
#include <QProcess>
#include <string>
#include <vector>
#include <filesystem>

#include "console_dialog.hpp"

class QComboBox;
class QPushButton;

namespace SixShooter {
    class MainWindow;
    
    class TagBludgeoner : public ConsoleDialog {
        Q_OBJECT
        friend class MainWindow;
        
    private:
        TagBludgeoner(const MainWindow *main_window);
        const MainWindow *main_window;
        
        QComboBox *tags;
        QComboBox *level;
        QPushButton *bludgeon_button;
        QProcess *process = nullptr;
        
        void reject() override;
        
        void set_ready(QProcess::ProcessState);
        void bludgeon_tags();
    };
}

#endif
