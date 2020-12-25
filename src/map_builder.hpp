// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_MAP_BUILDER_HPP
#define SIX_SHOOTER_MAP_BUILDER_HPP

#include <QDialog>
#include <QProcess>

#include "console_dialog.hpp"

class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;

namespace SixShooter {
    class MainWindow;
    
    class MapBuilder : public ConsoleDialog {
        Q_OBJECT
        friend class MainWindow;
    private:
        MapBuilder(const MainWindow *main_window);
        const MainWindow *main_window;
        ConsoleBox *console_box_stdout;
        ConsoleBox *console_box_stderr;
        QLineEdit *scenario_path;
        QComboBox *engine;
        QComboBox *compression;
        QCheckBox *optimize;
        QComboBox *raw_data;
        QPushButton *build_button;
        QProcess *process = nullptr;
        QLineEdit *index_path;
        QLineEdit *crc32;
        QLineEdit *rename_scenario;
        
        void keyPressEvent(QKeyEvent *e) override;
        void reject() override;
        
        void set_ready(QProcess::ProcessState);
        
        void compile_map();
        void find_index_path();
        void find_scenario_path();
        
        void restore_settings();
    };
}

#endif
