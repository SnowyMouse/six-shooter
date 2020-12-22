// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QProcess>

#include "console_box.hpp"
#include "map_builder.hpp"
#include "main_window.hpp"

namespace SixShooter {
    static const char *build_type[][2] = {
        {"Halo PC (Custom Edition)", "custom"},
        {"Halo PC (Retail)", "retail"},
        {"Halo PC (Demo)", "demo"},
        {"Halo PC (Custom Edition with CEA compression)", "mcc-custom"},
    };
    
    static const char *compression_type[][2] = {
        {"Automatic", nullptr},
        {"Compressed", "--compress"},
        {"Uncompressed", "--uncompressed"}
    };
    
    static const char *raw_data_type[][2] = {
        {"Automatic", nullptr},
        {"Self-contained", "--no-external-tags"},
        {"Always index (Custom Edition only)", "--always-index-tags"}
    };
    
    MapBuilder::MapBuilder(const MainWindow *main_window) : main_window(main_window) {
        auto *main_layout = new QHBoxLayout(this);
        this->setWindowTitle("Build a map - Six Shooter");
        
        // Add options on the left
        {
            auto *options_widget = new QGroupBox("Parameters", this);
            auto *options_main_layout_widget = new QWidget(options_widget);
            auto *options_main_layout = new QGridLayout(options_main_layout_widget);
            auto *options_layout = new QVBoxLayout(options_widget);
            options_main_layout->setMargin(0);
            
            // Scenario path
            this->scenario_path = new QLineEdit(options_widget);
            options_main_layout->addWidget(new QLabel("Scenario tag:"), 0, 0);
            options_main_layout->addWidget(this->scenario_path, 0, 1);
            
            // Map type
            this->engine = new QComboBox(options_widget);
            for(auto &i : build_type) {
                this->engine->addItem(i[0]);
            }
            options_main_layout->addWidget(new QLabel("Engine:", options_widget), 1, 0);
            options_main_layout->addWidget(this->engine, 1, 1);
            
            // Compression type
            this->compression = new QComboBox(options_widget);
            for(auto &i : compression_type) {
                this->compression->addItem(i[0]);
            }
            options_main_layout->addWidget(new QLabel("Compression:", options_widget), 2, 0);
            options_main_layout->addWidget(this->compression, 2, 1);
            
            // Compression type
            this->raw_data = new QComboBox(options_widget);
            for(auto &i : raw_data_type) {
                this->raw_data->addItem(i[0]);
            }
            options_main_layout->addWidget(new QLabel("External data:", options_widget), 3, 0);
            options_main_layout->addWidget(this->raw_data, 3, 1);
            
            options_main_layout_widget->setLayout(options_main_layout);
            options_layout->addWidget(options_main_layout_widget);
            
            // Index
            auto *index_path_finder = new QWidget(options_widget);
            auto *index_path_finder_layout = new QHBoxLayout(index_path_finder);
            index_path_finder_layout->setMargin(0);
            this->index_path = new QLineEdit(index_path_finder);
            index_path_finder_layout->addWidget(this->index_path);
            auto *find_index_button = new QPushButton("Find...", index_path_finder);
            index_path_finder_layout->addWidget(find_index_button);
            index_path_finder->setLayout(index_path_finder_layout);
            options_main_layout->addWidget(new QLabel("Index file:", options_widget), 4, 0);
            options_main_layout->addWidget(index_path_finder, 4, 1);
            connect(find_index_button, &QPushButton::clicked, this, &MapBuilder::find_index_path);
            
            // CRC32
            this->crc32 = new QLineEdit(options_widget);
            options_main_layout->addWidget(new QLabel("CRC32:", options_widget), 5, 0);
            options_main_layout->addWidget(this->crc32, 5, 1);
            
            // Dummy widget (spacing)
            auto *dummy_widget = new QWidget(options_widget);
            dummy_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
            options_layout->addWidget(dummy_widget);
            
            // Build button
            auto *compile_button = new QPushButton("Compile map", options_widget);
            connect(compile_button, &QPushButton::clicked, this, &MapBuilder::compile_map);
            options_layout->addWidget(compile_button);
            
            // Set the layout
            options_widget->setLayout(options_layout);
            
            options_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
            main_layout->addWidget(options_widget);
        }
        
        // Add a console on the right
        {
            auto *console_widget = new QWidget();
            auto *console_layout = new QVBoxLayout(console_widget);
            console_layout->setMargin(0);
            
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
    }
    
    void MapBuilder::compile_map() {
        if(this->process != nullptr) {
            delete this->process;
            this->process = nullptr;
        }
        
        // Process
        this->process = new QProcess(this);
        this->process->setProgram(this->main_window->executable_path("invader-build").string().c_str());
        
        // Set arguments
        QStringList arguments;
        for(auto &i : this->main_window->get_tags_directories()) {
            arguments << "--tags" << i.string().c_str();
        }
        
        arguments << "--maps" << this->main_window->get_maps_directory().string().c_str();
        arguments << "--game-engine" << build_type[this->engine->currentIndex()][1];
        
        auto *compressed = compression_type[this->compression->currentIndex()][1];
        if(compressed) {
            arguments << compressed;
        }
        
        auto *raw_data = raw_data_type[this->raw_data->currentIndex()][1];
        if(raw_data) {
            arguments << raw_data;
        }
        
        auto index_path = this->index_path->text();
        if(!index_path.isEmpty()) {
            arguments << "--with-index" << index_path;
        }
        
        auto crc32 = this->crc32->text();
        if(!crc32.isEmpty()) {
            arguments << "--forge-crc" << crc32;
        }
        
        arguments << this->scenario_path->text();
        
        // Invoke
        this->process->setArguments(arguments);
        this->console_box_stdout->attach_to_process(this->process, ConsoleBox::StandardOutput);
        this->console_box_stderr->attach_to_process(this->process, ConsoleBox::StandardError);
        this->process->start();
    }
    
    void MapBuilder::find_index_path() {
        QFileDialog qfd;
        qfd.setOptions(QFileDialog::Option::ReadOnly);
        qfd.setWindowTitle("Locate the index file");
        if(qfd.exec()) {
            this->index_path->setText(qfd.selectedFiles()[0]);
        }
    }
}
