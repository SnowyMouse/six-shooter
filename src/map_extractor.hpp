// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_MAP_EXTRACTOR_HPP
#define SIX_SHOOTER_MAP_EXTRACTOR_HPP

#include <QDialog>
#include <QProcess>
#include <string>
#include <vector>
#include <filesystem>

#include "console_dialog.hpp"

class QLineEdit;
class QComboBox;
class QCheckBox;
class QTreeWidget;
class QPushButton;
class QTreeWidgetItem;

namespace SixShooter {
    class MainWindow;
    class TagTreeWidget;
    
    class MapExtractor : public ConsoleDialog {
        Q_OBJECT
        friend class MainWindow;
    private:
        MapExtractor(const MainWindow *main_window, const std::filesystem::path &path);
        const MainWindow *main_window;
        
        const std::filesystem::path &path;
        
        QComboBox *tags;
        QProcess *process = nullptr;
        QLineEdit *map_path;
        
        TagTreeWidget *map_tags;
        QCheckBox *non_mp_globals;
        QCheckBox *overwrite;
        QCheckBox *ignore_resources;
        QCheckBox *use_maps_preferences;
        QPushButton *extract_button;
        
        void extract_full_map();
        void extract_map(const std::vector<std::string> &filter = std::vector<std::string>(), bool recursive = false, bool overwrite_anyway = false);
        void find_map_path();
        void reload_info();
        
        void set_ready(QProcess::ProcessState);
        
        QString get_map_info(const char *what) const;
        void reject() override;
        
        void double_clicked(QTreeWidgetItem *item, int column);
        void generate_index_file();
    };
}

#endif
