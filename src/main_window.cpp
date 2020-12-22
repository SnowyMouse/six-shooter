// SPDX-License-Identifier: GPL-3.0-only

#include <QStyle>
#include <QGuiApplication>
#include <QScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <filesystem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QProcess>

#include "map_builder.hpp"
#include "main_window.hpp"
#include "map_extractor.hpp"
#include "settings_editor.hpp"
#include "tag_bludgeoner.hpp"
#include "settings.hpp"

namespace SixShooter {
    MainWindow::MainWindow() {
        this->setWindowTitle("Six Shooter");
        
        // Reload these
        if(!this->reload_settings()) {
            this->start_settings_editor();
        }
        
        // Set up the GUI
        auto *window_widget = new QWidget(this);
        auto *window_layout = new QVBoxLayout(window_widget);
        
        // Map building
        {
            auto *map_building_box = new QGroupBox("Map building", window_widget);
            auto *map_building_layout = new QVBoxLayout(map_building_box);
            
            auto *map_builder = new QPushButton("Build a map", map_building_box);
            connect(map_builder, &QPushButton::clicked, this, &MainWindow::start_map_builder);
            map_building_layout->addWidget(map_builder);
            
            map_building_box->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            map_building_box->setLayout(map_building_layout);
            window_layout->addWidget(map_building_box);
        }
        
        // Tag editing options
        {
            auto *tag_editing_box = new QGroupBox("Tag editing", window_widget);
            auto *tag_editing_layout = new QVBoxLayout(tag_editing_box);
            
            auto *tag_editor_safe = new QPushButton("Launch invader-edit-qt", tag_editing_box);
            connect(tag_editor_safe, &QPushButton::clicked, this, &MainWindow::start_tag_editor_safe);
            tag_editing_layout->addWidget(tag_editor_safe);
            
            auto *tag_editor_unsafe = new QPushButton("Launch invader-edit-qt (Unsafe mode)", tag_editing_box);
            connect(tag_editor_unsafe, &QPushButton::clicked, this, &MainWindow::start_tag_editor_unsafe);
            tag_editing_layout->addWidget(tag_editor_unsafe);
            
            auto *tag_extractor = new QPushButton("Extract tags", tag_editing_box);
            connect(tag_extractor, &QPushButton::clicked, this, &MainWindow::start_tag_extractor);
            tag_editing_layout->addWidget(tag_extractor);
            
            auto *tag_bludgeoner = new QPushButton("Bludgeon tags", tag_editing_box);
            connect(tag_bludgeoner, &QPushButton::clicked, this, &MainWindow::start_tag_bludgeoner);
            tag_editing_layout->addWidget(tag_bludgeoner);
            
            tag_editing_box->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            tag_editing_box->setLayout(tag_editing_layout);
            window_layout->addWidget(tag_editing_box);
        }
        
        // Map building
        {
            auto *settings_box = new QGroupBox("Settings", window_widget);
            auto *settings_layout = new QVBoxLayout(settings_box);
            
            auto *settings_editor = new QPushButton("Edit settings", settings_box);
            connect(settings_editor, &QPushButton::clicked, this, &MainWindow::start_settings_editor);
            settings_layout->addWidget(settings_editor);
            
            settings_box->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            settings_box->setLayout(settings_layout);
            window_layout->addWidget(settings_box);
        }
        
        // Finish up
        window_widget->setLayout(window_layout);
        this->setCentralWidget(window_widget);
        this->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    }
    
    void MainWindow::start_tag_editor(bool disable_safeguards) {
        // Process
        QProcess process;
        process.setProgram(this->executable_path("invader-edit-qt").string().c_str());
        
        // Set arguments
        QStringList arguments;
        for(auto &i : this->tags_directories) {
            arguments << "--tags" << i.string().c_str();
        }
        if(disable_safeguards) {
            arguments << "--no-safeguards";
        }
        
        process.setArguments(arguments);
        
        // Begin
        if(!process.startDetached()) {
            std::printf("Failed to start invader-edit-qt... -.-\n");
        }
    }
    
    void MainWindow::start_tag_editor_safe() {
        return this->start_tag_editor(false);
    }
    
    void MainWindow::start_tag_editor_unsafe() {
        return this->start_tag_editor(true);
    }
    
    void MainWindow::start_tag_bludgeoner() {
        TagBludgeoner(this).exec();
    }
    
    void MainWindow::start_tag_extractor() {
        QFileDialog qfd;
        qfd.setFileMode(QFileDialog::FileMode::ExistingFile);
        qfd.setNameFilter("Maps (*.map)");
        qfd.setWindowTitle("Please open the map you want to extract");
        qfd.setDirectory(this->maps_directory.string().c_str());
        
        if(qfd.exec()) {
            MapExtractor(this, qfd.selectedFiles()[0].toStdString()).exec();
        }
    }
    
    void MainWindow::start_map_builder() {
        MapBuilder(this).exec();
    }
    
    bool MainWindow::reload_settings() {
        SixShooterSettings settings;
        
        this->invader_path = settings.value("invader_path").toString().toStdString();
        if(!invader_path_is_valid(this->invader_path)) {
            return false;
        }
        
        this->maps_directory = settings.value("maps_path").toString().toStdString();
        if(!std::filesystem::is_directory(this->maps_directory)) {
            return false;
        }
                
        this->tags_directories.clear();
        for(auto &i : settings.value("tags_directories").toStringList()) {
            std::filesystem::path path = i.toStdString();
            if(!std::filesystem::is_directory(path)) {
                return false;
            }
            this->tags_directories.emplace_back(path);
        }
        
        return true;
    }
    
    void MainWindow::start_settings_editor() {
        do { SettingsEditor(this, !this->isVisible()).exec(); } while (!this->reload_settings());
    }
    
    std::vector<std::filesystem::path> MainWindow::get_tags_directories() const {
        return this->tags_directories;
    }
    
    static std::filesystem::path executable_path(const std::filesystem::path &path, const char *executable) {
        const char *executable_extension = "";
        
        #ifdef _WIN32
        executable_extension = ".exe";
        #endif
        
        return path / (std::string(executable) + executable_extension);
    }
    
    bool MainWindow::invader_path_is_valid(const std::filesystem::path &path) {
        auto executable_exists = [&path](const char *executable) -> bool {
            return std::filesystem::exists(::SixShooter::executable_path(path, executable));
        };
        
        // Check if these exist
        return executable_exists("invader-build") && 
               executable_exists("invader-edit-qt") &&
               executable_exists("invader-extract") &&
               executable_exists("invader-info") &&
               executable_exists("invader-bludgeon");
    }
    
    std::filesystem::path MainWindow::executable_path(const char *executable) const {
        return ::SixShooter::executable_path(this->invader_path, executable);
    }
    
    std::filesystem::path MainWindow::get_maps_directory() const {
        return this->maps_directory;
    }
    
    std::filesystem::path MainWindow::get_invader_directory() const {
        return this->invader_path;
    }
}
