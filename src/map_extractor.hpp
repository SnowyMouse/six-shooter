// SPDX-License-Identifier: GPL-3.0-only

#include <QDialog>
#include <string>
#include <vector>
#include <filesystem>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QProcess;
class QTreeWidget;
class QTreeWidgetItem;

namespace SixShooter {
    class MainWindow;
    class ConsoleBox;
    class TagTreeWidget;
    
    class MapExtractor : public QDialog {
        Q_OBJECT
        friend class MainWindow;
    private:
        MapExtractor(const MainWindow *main_window, const std::filesystem::path &path);
        const MainWindow *main_window;
        
        const std::filesystem::path &path;
        
        QComboBox *tags;
        QProcess *process = nullptr;
        QLineEdit *map_path;
        ConsoleBox *console_box_stdout;
        ConsoleBox *console_box_stderr;
        
        TagTreeWidget *map_tags;
        QLineEdit *crc32;
        QCheckBox *non_mp_globals;
        QCheckBox *overwrite;
        QCheckBox *recursive;
        QCheckBox *ignore_resources;
        QCheckBox *use_maps_preferences;
        
        void extract_full_map();
        void extract_map(const std::vector<std::string> &filter = std::vector<std::string>());
        void find_map_path();
        void reload_info();
        
        QString get_map_info(const char *what) const;
        
        void double_clicked(QTreeWidgetItem *item, int column);
        
        ~MapExtractor();
    };
}
