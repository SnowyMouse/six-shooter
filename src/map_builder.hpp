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
class QLabel;

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
        QCheckBox *auto_forge;
        QCheckBox *optimize;
        QCheckBox *anniversary;
        QCheckBox *bypass_file_size_limits;
        QCheckBox *hide_pedantic_warnings;
        QComboBox *raw_data;
        QComboBox *script_source;
        QPushButton *build_button;
        QProcess *process = nullptr;
        QLineEdit *index_path;
        QLineEdit *build_string;
        QLineEdit *crc32;
        QLineEdit *rename_scenario;
        QLineEdit *dummy_build_string;
        
        QLabel *build_string_label;
        QLabel *raw_data_label;
        QLabel *crc32_label;
        QLabel *compression_label;
        QLabel *anniversary_label;
        QLabel *auto_forge_label;
        
        void keyPressEvent(QKeyEvent *e) override;
        void reject() override;
        
        void set_ready(QProcess::ProcessState);
        
        void compile_map();
        void find_index_path();
        void find_scenario_path();
        void toggle_build_string_visibility();
        
        void restore_settings();
    };
}

#endif
