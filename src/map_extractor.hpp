// SPDX-License-Identifier: GPL-3.0-only

#include <QDialog>
#include <string>
#include <vector>
#include <filesystem>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QProcess;

namespace SixShooter {
    class MainWindow;
    class ConsoleBox;
    
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
        
        QCheckBox *non_mp_globals;
        QCheckBox *overwrite;
        QCheckBox *recursive;
        
        void extract_full_map();
        void extract_map(const std::vector<std::string> &filter = std::vector<std::string>());
        void find_map_path();
        
        QString get_map_info(const char *what) const;
        
        ~MapExtractor();
    };
}
