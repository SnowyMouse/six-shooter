// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_MAP_BUILDER_HPP
#define SIX_SHOOTER_MAP_BUILDER_HPP

#include <QDialog>

class QLineEdit;
class QComboBox;
class QProcess;

namespace SixShooter {
    class MainWindow;
    class ConsoleBox;
    
    class MapBuilder : public QDialog {
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
        QComboBox *raw_data;
        QProcess *process = nullptr;
        QLineEdit *index_path;
        QLineEdit *crc32;
        
        void compile_map();
        void find_index_path();
        
        void restore_settings();
    };
}

#endif
