// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_MAIN_WINDOW_HPP
#define SIX_SHOOTER_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <filesystem>
#include <vector>

class QPushButton;

namespace SixShooter {
    class MainWindow : public QMainWindow {
        Q_OBJECT
        
    public:
        MainWindow();
        
        std::filesystem::path executable_path(const char *executable) const;
        std::vector<std::filesystem::path> get_tags_directories() const;
        std::filesystem::path get_maps_directory() const;
        std::filesystem::path get_invader_directory() const;
        
        static bool invader_path_is_valid(const std::filesystem::path &path);
        
        void show();
        
    private:
        bool reload_settings();
        
        void start_tag_editor(bool disable_safeguards);
        void start_tag_editor_safe();
        void start_tag_editor_unsafe();
        
        void start_map_builder();
        void start_tag_extractor();
        void start_settings_editor();
        void start_tag_bludgeoner();
        
        std::filesystem::path invader_path;
        std::filesystem::path maps_directory;
        std::vector<std::filesystem::path> tags_directories;
        
        QPushButton *invader_edit_qt_button;
        QPushButton *invader_edit_qt_unsafe_button;
        
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;
    };
}

#endif
