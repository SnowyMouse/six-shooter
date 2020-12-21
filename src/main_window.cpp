// SPDX-License-Identifier: GPL-3.0-only

#include <QStyle>
#include <QGuiApplication>
#include <QScreen>
#include <QSettings>
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

namespace SixShooter {
    static bool invader_path_is_valid(const std::filesystem::path &path);
    
    MainWindow::MainWindow() {
        this->setWindowTitle("Six Shooter");
        
        // Get settings
        QSettings settings;
        
        // Do we have Invader present?
        if(settings.value("invader_path").isNull()) {
            if(!find_invader()) {
                std::exit(EXIT_FAILURE);
            }
        }
        else {
            if(!invader_path_is_valid(settings.value("invader_path").toString().toStdString())) {
                QMessageBox qmd;
                qmd.setWindowTitle("Path error");
                qmd.setText("The Invader path stored in the application config is no longer valid. You will have to re-find it.");
                qmd.setIcon(QMessageBox::Icon::Warning);
                qmd.exec();
                if(!find_invader()) {
                    std::exit(EXIT_FAILURE);
                }
            }
        }
        this->invader_path = settings.value("invader_path").toString().toStdString();
        
        // Do we have tag directories?
        this->reload_tag_directories();
        
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
    
    bool MainWindow::find_invader() {
        while(true) {
            // Initialize the file dialog
            QFileDialog qfd;
            qfd.setOptions(QFileDialog::Option::ReadOnly | QFileDialog::Option::ShowDirsOnly);
            qfd.setWindowTitle("Please find the folder where Invader is located");
            if(qfd.exec()) {
                // Look for it!
                auto path = qfd.selectedFiles()[0];
                if(invader_path_is_valid(path.toStdString())) {
                    QSettings().setValue("invader_path", path);
                    return true;
                }
                else {
                    QMessageBox qmd;
                    qmd.setWindowTitle("Path error");
                    qmd.setText("The path (" + path + ") is not valid.");
                    qmd.setIcon(QMessageBox::Icon::Critical);
                    qmd.exec();
                    continue;
                }
            }
            else {
                return false;
            }
        }
    }
    
    void MainWindow::reload_tag_directories() {
        this->tag_directories.clear();
        
        QSettings settings;
        auto setting = settings.value("tags_directories");
        QStringList directory_list;
        
        // Load it!
        if(setting.isNull() || setting.toStringList().size() == 0) {
            // Initialize the file dialog
            QFileDialog qfd;
            qfd.setOptions(QFileDialog::Option::ReadOnly | QFileDialog::Option::ShowDirsOnly);
            qfd.setWindowTitle("Please locate your tags directory");
            if(qfd.exec()) {
                directory_list = qfd.selectedFiles();
                settings.setValue("tags_directories", directory_list);
            }
            else {
                std::exit(EXIT_FAILURE);
            }
        }
        else {
            directory_list = setting.toStringList();
        }
        
        // Add it!
        for(auto &i : directory_list) {
            this->tag_directories.emplace_back(i.toStdString());
        }
    }
    
    void MainWindow::start_tag_editor(bool disable_safeguards) {
        // Process
        QProcess process;
        process.setProgram(this->executable_path("invader-edit-qt").string().c_str());
        
        // Set arguments
        QStringList arguments;
        for(auto &i : this->tag_directories) {
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
    
    void MainWindow::start_tag_extractor() {
        std::printf("stub\n");
    }
    
    void MainWindow::start_map_builder() {
        MapBuilder builder(this);
        builder.exec();
    }
    
    void MainWindow::start_settings_editor() {
        std::printf("stub\n");
    }
    
    std::vector<std::filesystem::path> MainWindow::get_tag_directories() const {
        return this->tag_directories;
    }
    
    static std::filesystem::path executable_path(const std::filesystem::path &path, const char *executable) {
        const char *executable_extension = "";
        
        #ifdef _WIN32
        executable_extension = ".exe";
        #endif
        
        return path / (std::string(executable) + executable_extension);
    }
    
    static bool invader_path_is_valid(const std::filesystem::path &path) {
        auto executable_exists = [&path](const char *executable) -> bool {
            return std::filesystem::exists(executable_path(path, executable));
        };
        
        // Check if these exist
        return executable_exists("invader-build") && 
               executable_exists("invader-edit-qt") &&
               executable_exists("invader-extract") &&
               executable_exists("invader-info");
    }
    
    std::filesystem::path MainWindow::executable_path(const char *executable) const {
        return ::SixShooter::executable_path(this->invader_path, executable);
    }
}
