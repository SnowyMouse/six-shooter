// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_SETTINGS_EDITOR_HPP
#define SIX_SHOOTER_SETTINGS_EDITOR_HPP

#include <QDialog>
#include <filesystem>
#include <vector>

class QLineEdit;
class QTableWidget;

namespace SixShooter {
    class MainWindow;
    
    class SettingsEditor : public QDialog {
        Q_OBJECT
        friend class MainWindow;
    private:
        SettingsEditor(MainWindow *main_window, bool exit_on_failure);
        
        MainWindow *main_window;
        
        class Finder;
        Finder *invader;
        Finder *maps;
        Finder *data;
        
        QTableWidget *tags;
        std::vector<std::filesystem::path> tags_paths;
        
        void save_settings();
        void reject() override;
        void accept() override;
        bool exit_on_failure;
        
        void refresh_tags_table();
        
        void move_up();
        void move_down();
        void remove_path();
        void add_path();
        void modified_path(int row, int column);
    };
}

#endif
