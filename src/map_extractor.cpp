// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QProcess>
#include <QLineEdit>
#include <QTreeWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QFileIconProvider>

#include "console_box.hpp"
#include "main_window.hpp"
#include "map_extractor.hpp"
#include "tag_tree_widget.hpp"

namespace SixShooter {
    MapExtractor::MapExtractor(const MainWindow *main_window, const std::filesystem::path &path) : main_window(main_window), path(path) {
        auto *main_layout = new QHBoxLayout(this);
        this->setWindowTitle("Extract a map - Six Shooter");
        
        auto *left_widget = new QWidget(this);
        auto *left_layout = new QVBoxLayout(left_widget);
        left_layout->setContentsMargins(0, 0, 0, 0);
        
        // Add options on the left
        {
            auto *options_widget = new QGroupBox("Parameters", left_widget);
            auto *options_layout = new QGridLayout(options_widget);
            
            // Add item
            this->tags = new QComboBox(options_widget);
            for(auto &i : this->main_window->get_tags_directories()) {
                this->tags->insertItem(0, i.string().c_str());
            }
            this->tags->setMinimumWidth(400);
            this->tags->setMaximumWidth(400);
            
            options_layout->addWidget(new QLabel("Tags directory:"), 0, 0);
            options_layout->addWidget(this->tags, 0, 1);
            
            this->non_mp_globals = new QCheckBox(options_widget);
            options_layout->addWidget(new QLabel("Extract non-multiplayer globals:"), 1, 0);
            options_layout->addWidget(this->non_mp_globals, 1, 1);
            
            this->recursive = new QCheckBox(options_widget);
            options_layout->addWidget(new QLabel("Recursive:"), 2, 0);
            options_layout->addWidget(this->recursive, 2, 1);
            
            this->overwrite = new QCheckBox(options_widget);
            options_layout->addWidget(new QLabel("Overwrite:"), 3, 0);
            options_layout->addWidget(this->overwrite, 3, 1);
            
            this->ignore_resources = new QCheckBox(options_widget);
            options_layout->addWidget(new QLabel("Ignore external tags:"), 4, 0);
            options_layout->addWidget(this->ignore_resources, 4, 1);
            
            this->use_maps_preferences = new QCheckBox(options_widget);
            options_layout->addWidget(new QLabel("Use maps folder from preferences:"), 5, 0);
            options_layout->addWidget(this->use_maps_preferences, 5, 1);
            
            // Set the layout
            options_widget->setLayout(options_layout);
            
            options_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            left_layout->addWidget(options_widget);
        }
        
        // Some metdata
        {
            auto *metadata_widget = new QGroupBox("Metadata", left_widget);
            auto *metadata_layout = new QGridLayout(metadata_widget);
            
            this->crc32 = new QLineEdit(metadata_widget);
            this->crc32->setReadOnly(true);
            metadata_layout->addWidget(new QLabel("CRC32:"), 0, 0);
            metadata_layout->addWidget(this->crc32, 0, 1);
            
            metadata_widget->setLayout(metadata_layout);
            metadata_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            left_layout->addWidget(metadata_widget);
        }
        
        // Tags
        {
            auto *tags_widget = new QGroupBox("Tags", left_widget);
            auto *tags_layout = new QVBoxLayout(tags_widget);
            
            this->map_tags = new TagTreeWidget(tags_widget);
            connect(this->map_tags, &QTreeWidget::itemDoubleClicked, this, &MapExtractor::double_clicked);
            tags_layout->addWidget(this->map_tags);
            
            // Extract button
            auto *extract_button = new QPushButton("Extract all tags", tags_widget);
            connect(extract_button, &QPushButton::clicked, this, &MapExtractor::extract_full_map);
            tags_layout->addWidget(extract_button);
            
            tags_widget->setLayout(tags_layout);
            left_layout->addWidget(tags_widget);
        }
        
        left_widget->setLayout(left_layout);
        main_layout->addWidget(left_widget);
        
        // Add a console on the right
        {
            auto *console_widget = new QWidget();
            auto *console_layout = new QVBoxLayout(console_widget);
            console_layout->setContentsMargins(0, 0, 0, 0);
            
            auto *stdout_widget = new QGroupBox("Output", this);
            auto *stdout_layout = new QVBoxLayout(stdout_widget);
            this->console_box_stdout = new ConsoleBox(console_widget);
            stdout_layout->addWidget(this->console_box_stdout);
            stdout_widget->setLayout(stdout_layout);
            console_layout->addWidget(stdout_widget);
            
            auto *stderr_widget = new QGroupBox("Errors", this);
            auto *stderr_layout = new QVBoxLayout(stderr_widget);
            this->console_box_stderr = new ConsoleBox(console_widget);
            stderr_layout->addWidget(this->console_box_stderr);
            stderr_widget->setLayout(stderr_layout);
            console_layout->addWidget(stderr_widget);
            
            console_widget->setLayout(console_layout);
            console_widget->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
            main_layout->addWidget(console_widget);
        }
        
        this->reload_info();
    }
    
    MapExtractor::~MapExtractor() {
        if(this->process != nullptr) {
            this->process->kill();
        }
    }
    
    void MapExtractor::extract_map(const std::vector<std::string> &filter) {
        if(this->process != nullptr) {
            this->process->kill();
            delete this->process;
            this->process = nullptr;
        }
        
        // Process
        this->process = new QProcess(this);
        this->process->setProgram(this->main_window->executable_path("invader-extract").string().c_str());
        
        // Set arguments
        QStringList arguments;
        arguments << "--tags" << this->tags->currentText();
        
        if(this->use_maps_preferences->isChecked()) {
            arguments << "--maps" << this->main_window->get_maps_directory().string().c_str();
        }
        
        arguments << this->path.string().c_str();
        
        if(this->non_mp_globals->isChecked()) {
            arguments << "--non-mp-globals";
        }
        
        if(this->recursive->isChecked()) {
            arguments << "--recursive";
        }
        
        if(this->overwrite->isChecked()) {
            arguments << "--overwrite";
        }
        
        if(this->ignore_resources->isChecked()) {
            arguments << "--ignore-resources";
        }
        
        for(auto &i : filter) {
            arguments << "--search" << i.c_str();
        }
        
        // Invoke
        this->process->setArguments(arguments);
        this->console_box_stdout->attach_to_process(this->process, ConsoleBox::StandardOutput);
        this->console_box_stderr->attach_to_process(this->process, ConsoleBox::StandardError);
        this->process->start();
    }
    
    void MapExtractor::extract_full_map() {
        return extract_map();
    }
    
    QString MapExtractor::get_map_info(const char *what) const {
        // Process
        QProcess process;
        process.setProgram(this->main_window->executable_path("invader-info").string().c_str());
        
        QStringList arguments;
        arguments << "--type" << what;
        arguments << this->path.string().c_str();
        process.setArguments(arguments);
        
        process.start();
        process.waitForFinished(-1);
        
        return QString(process.readAllStandardOutput()).trimmed();
    }
    
    void MapExtractor::reload_info() {
        this->crc32->setText(get_map_info("crc32"));
        this->map_tags->clear();
        
        auto tags = get_map_info("tags").split("\n");
        this->map_tags->set_data(tags);
    }
    
    void MapExtractor::double_clicked(QTreeWidgetItem *item, int column) {
        auto data = item->data(column, Qt::UserRole);
        if(!data.isNull()) {
            std::vector<std::string> filters;
            filters.emplace_back(data.toString().toStdString());
            this->extract_map(filters);
        }
    }
}
