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
        
        std::filesystem::path invader_path;
        std::vector<std::filesystem::path> tag_directories;
        
        static bool invader_path_is_valid(const std::filesystem::path &path);
    };
}
