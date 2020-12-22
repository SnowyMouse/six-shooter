// SPDX-License-Identifier: GPL-3.0-only

#include <QMainWindow>
#include <filesystem>
#include <vector>

namespace SixShooter {
    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        MainWindow();
        
        std::filesystem::path executable_path(const char *executable) const;
        std::vector<std::filesystem::path> get_tags_directories() const;
        std::filesystem::path get_maps_directory() const;
        
    private:
        bool find_invader();
        bool find_maps();
        
        void reload_tags_directories();
        void reload_maps_directory();
        void reload_invader_directory();
        
        void start_tag_editor(bool disable_safeguards);
        void start_tag_editor_safe();
        void start_tag_editor_unsafe();
        
        void start_map_builder();
        void start_tag_extractor();
        void start_settings_editor();
        
        std::filesystem::path invader_path;
        std::filesystem::path maps_directory;
        std::vector<std::filesystem::path> tags_directories;
    };
}
