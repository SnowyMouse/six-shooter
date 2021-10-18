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
#include <QMessageBox>
#include <QProcess>
#include <QCheckBox>
#include <QKeyEvent>

#include "tag_tree_dialog.hpp"
#include "console_box.hpp"
#include "map_builder.hpp"
#include "main_window.hpp"
#include "settings.hpp"

namespace SixShooter {
    static const char *build_type[][2] = {
        {"MCC (CEA)", "mcc-cea"},
        {"Halo PC (Custom Edition)", "gbx-custom"},
        {"Halo PC (Retail)", "gbx-retail"},
        {"Halo PC (Demo)", "gbx-demo"},
        {"Halo Xbox (English NTSC)", "xbox-ntsc"},
        {"Halo Xbox (English PAL)", "xbox-pal"},
        {"Halo Xbox (Japanese)", "xbox-ntsc-jp"},
        {"Halo Xbox (Taiwanese)", "xbox-ntsc-tw"},
    };
    
    static const char *compression_type[] = {
        "Compressed (Best)", "Compressed (Good)", "Compressed (Fast)", "Compressed (Fastest)"
    };
    
    static const char *raw_data_type[][2] = {
        {"Automatic", "check"},
        {"Self-contained", "none"},
        {"Always index (Custom Edition only)", "always"}
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
            scenario_path_container_layout->addWidget((this->scenario_path = new QLineEdit(scenario_path_container)));
            scenario_path_container_layout->addWidget(find_scenario_button);
            scenario_path_container->setLayout(scenario_path_container_layout);
            options_main_layout->addWidget(new QLabel("Scenario tag:"), 0, 0);
            options_main_layout->addWidget(scenario_path_container, 0, 1);
            
            // Map type
            options_main_layout->addWidget(new QLabel("Engine:", options_widget), 1, 0);
            this->engine = new QComboBox(options_widget);
            for(auto &i : build_type) {
                this->engine->addItem(i[0]);
            }
            options_main_layout->addWidget(this->engine, 1, 1);
            
            connect(this->engine, &QComboBox::currentTextChanged, this, &MapBuilder::toggle_build_string_visibility);
            
            // Compression type
            options_main_layout->addWidget((this->compression_label = new QLabel("Compression:", options_widget)), 2, 0);
            this->compression = new QComboBox(options_widget);
            for(auto &i : compression_type) {
                this->compression->addItem(i);
            }
            options_main_layout->addWidget(this->compression, 2, 1);
            
            // Compression type
            options_main_layout->addWidget((this->raw_data_label = new QLabel("External data:", options_widget)), 3, 0);
            this->raw_data = new QComboBox(options_widget);
            for(auto &i : raw_data_type) {
                this->raw_data->addItem(i[0]);
            }
            options_main_layout->addWidget(this->raw_data, 3, 1);
            
            options_main_layout_widget->setLayout(options_main_layout);
            options_layout->addWidget(options_main_layout_widget);
            
            // Index
            auto *index_path_finder = new QWidget(options_widget);
            auto *index_path_finder_layout = new QHBoxLayout(index_path_finder);
            index_path_finder_layout->setContentsMargins(0, 0, 0, 0);
            index_path_finder_layout->addWidget((this->index_path = new QLineEdit(index_path_finder)));
            auto *find_index_button = new QPushButton("Find...", index_path_finder);
            index_path_finder_layout->addWidget(find_index_button);
            index_path_finder->setLayout(index_path_finder_layout);
            options_main_layout->addWidget(new QLabel("Index file:", options_widget), 4, 0);
            options_main_layout->addWidget(index_path_finder, 4, 1);
            connect(find_index_button, &QPushButton::clicked, this, &MapBuilder::find_index_path);
            this->index_path->setPlaceholderText("None");
            
            // Scenario
            options_main_layout->addWidget(new QLabel("Rename scenario:", options_widget), 5, 0);
            options_main_layout->addWidget((this->rename_scenario = new QLineEdit(options_widget)), 5, 1);
            this->rename_scenario->setPlaceholderText("None");
            
            // CRC32
            options_main_layout->addWidget((this->crc32_label = new QLabel("Forge CRC32:", options_widget)), 6, 0);
            options_main_layout->addWidget((this->crc32 = new QLineEdit(options_widget)), 6, 1);
            this->crc32->setPlaceholderText("None");
            
            // Build string
            options_main_layout->addWidget((this->build_string_label = new QLabel("Build string:", options_widget)), 7, 0);
            options_main_layout->addWidget((this->build_string = new QLineEdit(options_widget)), 7, 1);
            options_main_layout->addWidget((this->dummy_build_string = new QLineEdit(options_widget)), 7, 1);
            
            // Use CEA things
            options_main_layout->addWidget((this->anniversary_label = new QLabel("Enable \"anniversary\" mode:", options_widget)), 8, 0);
            options_main_layout->addWidget((this->anniversary = new QCheckBox(options_widget)), 8, 1);
            
            // Automatically forge tag indices
            options_main_layout->addWidget((this->auto_forge_label = new QLabel("Auto-forge tag indices:", options_widget)), 9, 0);
            options_main_layout->addWidget((this->auto_forge = new QCheckBox(options_widget)), 9, 1);
            
            // Tag space
            options_main_layout->addWidget(new QLabel("Optimize tag space:", options_widget), 10, 0);
            options_main_layout->addWidget((this->optimize = new QCheckBox(options_widget)), 10, 1);
            
            // File size
            options_main_layout->addWidget(new QLabel("Bypass file size limits:", options_widget), 11, 0);
            options_main_layout->addWidget((this->bypass_file_size_limits = new QCheckBox(options_widget)), 11, 1);
            
            // Pedantic warnings
            options_main_layout->addWidget(new QLabel("Hide pedantic warnings:", options_widget), 12, 0);
            options_main_layout->addWidget((this->hide_pedantic_warnings = new QCheckBox(options_widget)), 12, 1);
            
            // Dummy widget (spacing)
            auto *dummy_widget = new QWidget(options_widget);
            dummy_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
            options_layout->addWidget(dummy_widget);
            
            // Build button;
            options_layout->addWidget((this->build_button = new QPushButton("Compile map", options_widget)));
            connect(this->build_button, &QPushButton::clicked, this, &MapBuilder::compile_map);
            
            // Set the layout
            options_widget->setLayout(options_layout);
            
            options_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
            main_layout->addWidget(options_widget);
        }
        
        // Add a console on the right
        main_layout->addWidget(this->get_console_widget());
        
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
        connect(this->process, &QProcess::stateChanged, this, &MapBuilder::set_ready);
        
        // Set arguments
        QStringList arguments;
        for(auto &i : this->main_window->get_tags_directories()) {
            arguments << "--tags" << i.string().c_str();
        }
        
        SixShooterSettings settings;
        
        arguments << "--maps" << this->main_window->get_maps_directory().string().c_str();
        
        arguments << "--game-engine" << build_type[this->engine->currentIndex()][1];
        settings.setValue("last_compiled_scenario_engine", this->engine->currentText());
        
        // Xbox?
        bool is_xbox = this->engine->currentIndex() >= 4;
        if(is_xbox) {
            auto build_string = this->build_string->text();
            settings.setValue("last_compiled_build_string", build_string);
            if(build_string != "") {
                arguments << "--build-string" << build_string;
            }
            
            auto compressed = this->compression->currentIndex();
            switch(compressed) {
                // Best
                case 0:
                    arguments << "--level" << "9";
                    break;
                // Good
                case 1:
                    arguments << "--level" << "6";
                    break;
                // Fast
                case 2:
                    arguments << "--level" << "3";
                    break;
                // Fastest
                case 3:
                    arguments << "--level" << "1";
                    break;
            }
        }
        settings.setValue("last_compiled_scenario_compressed", this->compression->currentText());
        
        // CEA stuff
        bool is_cea = this->engine->currentIndex() == 0;
        
        auto *raw_data = raw_data_type[this->raw_data->currentIndex()][1];
        if(raw_data && !is_xbox) {
            arguments << "--resource-usage" << raw_data;
        }
        settings.setValue("last_compiled_scenario_raw_data", this->raw_data->currentText());
        
        auto index_path = this->index_path->text();
        if(!index_path.isEmpty()) {
            arguments << "--with-index" << index_path;
        }
        settings.setValue("last_compiled_scenario_index", index_path);
        
        auto anniversary = this->anniversary->isChecked();
        if(is_cea && anniversary) {
            arguments << "--anniversary-mode";
        }
        settings.setValue("last_compiled_anniversary", anniversary);
        
        auto scenario_name = this->rename_scenario->text();
        if(!scenario_name.isEmpty()) {
            arguments << "--rename-scenario" << scenario_name;
        }
        settings.setValue("last_compiled_scenario_name", scenario_name);
        
        auto crc32 = this->crc32->text();
        if(!crc32.isEmpty() && !is_xbox) {
            arguments << "--forge-crc" << crc32;
        }
        settings.setValue("last_compiled_scenario_crc32", crc32);
        
        QString scenario = this->scenario_path->text();
        arguments << this->scenario_path->text();
        settings.setValue("last_compiled_scenario", scenario);
        
        auto auto_forge = this->auto_forge->isChecked();
        if(!is_xbox && auto_forge) {
            arguments << "--auto-forge";
        }
        settings.setValue("last_compiled_auto_forge", auto_forge);
        
        bool optimize = this->optimize->isChecked();
        if(optimize) {
            arguments << "--optimize";
        }
        settings.setValue("last_compiled_optimize", optimize);
        
        bool bypass_file_size_limits = this->bypass_file_size_limits->isChecked();
        if(bypass_file_size_limits) {
            arguments << "--extend-file-limits";
        }
        settings.setValue("last_compiled_extended_file_size", bypass_file_size_limits);
        
        bool hide_pedantic_warnings = this->hide_pedantic_warnings->isChecked();
        if(hide_pedantic_warnings) {
            arguments << "--hide-pedantic-warnings";
        }
        settings.setValue("last_compiled_hide_pedantic_warnings", hide_pedantic_warnings);
        
        // Invoke
        this->process->setArguments(arguments);
        this->attach_to_process(this->process);
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
        this->auto_forge->setChecked(settings.value("last_compiled_auto_forge", false).toBool());
        this->optimize->setChecked(settings.value("last_compiled_optimize", false).toBool());
        this->bypass_file_size_limits->setChecked(settings.value("last_compiled_extended_file_size", false).toBool());
        this->hide_pedantic_warnings->setChecked(settings.value("last_compiled_hide_pedantic_warnings", false).toBool());
        this->rename_scenario->setText(settings.value("last_compiled_scenario_name", QString("")).toString());
        this->build_string->setText(settings.value("last_compiled_build_string", QString("")).toString());
        this->anniversary->setChecked(settings.value("last_compiled_anniversary", false).toBool());
        this->toggle_build_string_visibility();
    }
    
    void MapBuilder::toggle_build_string_visibility() {
        bool is_cea = this->engine->currentIndex() == 0;
        bool is_xbox = this->engine->currentIndex() >= 4;
        
        this->build_string->setVisible(is_xbox);
        this->dummy_build_string->setVisible(!is_xbox);
        this->build_string_label->setEnabled(is_xbox);
        
        this->raw_data->setEnabled(!is_xbox);
        this->raw_data_label->setEnabled(!is_xbox);
        
        this->crc32->setEnabled(!is_xbox);
        this->crc32_label->setEnabled(!is_xbox);
        
        this->compression->setEnabled(is_xbox);
        this->compression_label->setEnabled(is_xbox);
        
        this->anniversary->setEnabled(is_cea);
        this->anniversary_label->setEnabled(is_cea);

        this->auto_forge->setEnabled(!is_xbox);
        this->auto_forge_label->setEnabled(!is_xbox);
    }
    
    void MapBuilder::find_index_path() {
        QFileDialog qfd;
        qfd.setFileMode(QFileDialog::FileMode::ExistingFile);
        qfd.setWindowTitle("Locate the index file");
        qfd.setNameFilter("Text file (*.txt)");
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
            this->scenario_path->setText(std::filesystem::path(dialog.get_result().toStdString()).replace_extension("").string().c_str());
        }
    }
    
    void MapBuilder::keyPressEvent(QKeyEvent *e) {
        switch(e->key()) {
            case Qt::Key::Key_Enter:
            case Qt::Key::Key_Return:
                this->compile_map();
                e->accept();
                return;
            default:
                QDialog::keyPressEvent(e);
        }
    }
    
    void MapBuilder::set_ready(QProcess::ProcessState state) {
        this->build_button->setEnabled(state == QProcess::ProcessState::NotRunning);
    }
    
    void MapBuilder::reject() {
        if(this->process) {
            if(this->process->state() == QProcess::ProcessState::Running) {
                QMessageBox qmb;
                qmb.setWindowTitle("Map compilation in progress");
                qmb.setText("Are you sure you want to stop building the map?");
                qmb.setStandardButtons(QMessageBox::StandardButton::Abort | QMessageBox::StandardButton::Cancel);
                qmb.setIcon(QMessageBox::Icon::Question);
                if(qmb.exec() == QMessageBox::StandardButton::Cancel) {
                    return;
                }
                this->process->kill();
                this->process->waitForFinished(-1);
            }
            delete this->process;
            this->process = nullptr;
        }
        QDialog::reject();
    }
}
