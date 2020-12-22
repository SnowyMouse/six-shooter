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
#include <QCheckBox>

#include "tag_tree_dialog.hpp"
#include "console_box.hpp"
#include "map_builder.hpp"
#include "main_window.hpp"
#include "settings.hpp"

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
            options_main_layout->setContentsMargins(0, 0, 0, 0);
            
            // Scenario path
            auto *scenario_path_container = new QWidget(options_widget);
            auto *scenario_path_container_layout = new QHBoxLayout(scenario_path_container);
            scenario_path_container_layout->setContentsMargins(0, 0, 0, 0);
            auto *find_scenario_button = new QPushButton("Find...", scenario_path_container);
            connect(find_scenario_button, &QPushButton::clicked, this, &MapBuilder::find_scenario_path);
            this->scenario_path = new QLineEdit(scenario_path_container);
            scenario_path_container_layout->addWidget(this->scenario_path);
            scenario_path_container_layout->addWidget(find_scenario_button);
            scenario_path_container->setLayout(scenario_path_container_layout);
            options_main_layout->addWidget(new QLabel("Scenario tag:"), 0, 0);
            options_main_layout->addWidget(scenario_path_container, 0, 1);
            
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
            index_path_finder_layout->setContentsMargins(0, 0, 0, 0);
            this->index_path = new QLineEdit(index_path_finder);
            this->index_path->setPlaceholderText("None");
            index_path_finder_layout->addWidget(this->index_path);
            auto *find_index_button = new QPushButton("Find...", index_path_finder);
            index_path_finder_layout->addWidget(find_index_button);
            index_path_finder->setLayout(index_path_finder_layout);
            options_main_layout->addWidget(new QLabel("Index file:", options_widget), 4, 0);
            options_main_layout->addWidget(index_path_finder, 4, 1);
            connect(find_index_button, &QPushButton::clicked, this, &MapBuilder::find_index_path);
            
            // CRC32
            this->crc32 = new QLineEdit(options_widget);
            this->crc32->setPlaceholderText("None");
            options_main_layout->addWidget(new QLabel("CRC32:", options_widget), 5, 0);
            options_main_layout->addWidget(this->crc32, 5, 1);
            
            // CRC32
            this->optimize = new QCheckBox(options_widget);
            options_main_layout->addWidget(new QLabel("Optimize tag space:", options_widget), 6, 0);
            options_main_layout->addWidget(this->optimize, 6, 1);
            
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
        
        this->restore_settings();
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
        
        SixShooterSettings settings;
        
        arguments << "--maps" << this->main_window->get_maps_directory().string().c_str();
        
        arguments << "--game-engine" << build_type[this->engine->currentIndex()][1];
        settings.setValue("last_compiled_scenario_engine", this->engine->currentText());
        
        auto *compressed = compression_type[this->compression->currentIndex()][1];
        if(compressed) {
            arguments << compressed;
        }
        settings.setValue("last_compiled_scenario_compressed", this->compression->currentText());
        
        auto *raw_data = raw_data_type[this->raw_data->currentIndex()][1];
        if(raw_data) {
            arguments << raw_data;
        }
        settings.setValue("last_compiled_scenario_raw_data", this->raw_data->currentText());
        
        auto index_path = this->index_path->text();
        if(!index_path.isEmpty()) {
            arguments << "--with-index" << index_path;
        }
        settings.setValue("last_compiled_scenario_index", index_path);
        
        auto crc32 = this->crc32->text();
        if(!crc32.isEmpty()) {
            arguments << "--forge-crc" << crc32;
        }
        settings.setValue("last_compiled_scenario_crc32", crc32);
        
        QString scenario = this->scenario_path->text();
        arguments << this->scenario_path->text();
        settings.setValue("last_compiled_scenario", scenario);
        
        bool optimize = this->optimize->isChecked();
        if(optimize) {
            arguments << "--optimize";
        }
        settings.setValue("last_compiled_optimize", optimize);
        
        // Invoke
        this->process->setArguments(arguments);
        this->console_box_stdout->attach_to_process(this->process, ConsoleBox::StandardOutput);
        this->console_box_stderr->attach_to_process(this->process, ConsoleBox::StandardError);
        this->process->start();
    }
    
    void MapBuilder::restore_settings() {
        SixShooterSettings settings;
        this->engine->setCurrentText(settings.value("last_compiled_scenario_engine", QString("")).toString());
        this->compression->setCurrentText(settings.value("last_compiled_scenario_compressed", QString("")).toString());
        this->raw_data->setCurrentText(settings.value("last_compiled_scenario_raw_data", QString("")).toString());
        this->index_path->setText(settings.value("last_compiled_scenario_index", QString("")).toString());
        this->crc32->setText(settings.value("last_compiled_scenario_crc32", QString("")).toString());
        this->scenario_path->setText(settings.value("last_compiled_scenario", QString("")).toString());
        this->optimize->setChecked(settings.value("last_compiled_optimize", false).toBool());
    }
    
    void MapBuilder::find_index_path() {
        QFileDialog qfd;
        qfd.setFileMode(QFileDialog::FileMode::ExistingFile);
        qfd.setWindowTitle("Locate the index file");
        if(qfd.exec()) {
            this->index_path->setText(qfd.selectedFiles()[0]);
        }
    }
    
    void MapBuilder::find_scenario_path() {
        // Get all the tags
        QStringList list;
        
        for(auto &i : this->main_window->get_tags_directories()) {
            auto traverse_directory = [&list, &i](const std::filesystem::path &directory, auto &recursion) -> void {
                try {
                    for(auto &f : std::filesystem::directory_iterator(directory)) {
                        if(std::filesystem::is_directory(f)) {
                            recursion(f, recursion);
                        }
                        else if(std::filesystem::is_regular_file(f)) {
                            auto relative_path = std::filesystem::relative(f, i);
                            if(relative_path.extension() != ".scenario") {
                                continue;
                            }
                            
                            auto relative_path_str = QString(relative_path.string().c_str());
                            if(!list.contains(relative_path_str)) {
                                list << relative_path_str;
                            }
                        }
                    }
                }
                catch (std::exception &e) {
                    std::fprintf(stderr, "Failed to query %s: %s\n", directory.string().c_str(), e.what());
                }
            };
            traverse_directory(i, traverse_directory);
        }
        
        TagTreeDialog dialog;
        dialog.setWindowTitle("Locate the scenario tag - Six Shooter");
        dialog.set_data(list);
        if(dialog.exec()) {
            this->scenario_path->setText(std::filesystem::path(dialog.get_result().toStdString()).stem().string().c_str());
        }
    }
}
