// SPDX-License-Identifier: GPL-3.0-only

#include <QMainWindow>
#include <filesystem>

namespace SixShooter {
    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        MainWindow();
        
    private:
        bool find_invader();
        static bool invader_path_is_valid(const std::filesystem::path &path);
        
        std::filesystem::path invader_path;
    };
}
