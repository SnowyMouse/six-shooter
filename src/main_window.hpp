// SPDX-License-Identifier: GPL-3.0-only

#include <QMainWindow>
#include <filesystem>
#include <vector>

namespace SixShooter {
    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        MainWindow();
        
    private:
        bool find_invader();
        void reload_tag_directories();
        
        void start_tag_editor(bool safe);
        void start_tag_editor_safe();
        void start_tag_editor_unsafe();
        
        std::filesystem::path executable_path(const char *executable);
        
        std::filesystem::path invader_path;
        std::vector<std::filesystem::path> tag_directories;
    };
}
