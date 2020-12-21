// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QProcess>

#include "console_box.hpp"
#include "map_builder.hpp"
#include "main_window.hpp"

namespace SixShooter {
    static const char *build_type[][2] = {
        {"Halo: Custom Edition", "custom"},
        {"Halo (Retail Version)", "retail"},
        {"Halo (Demo Version)", "demo"},
        {"Halo: Custom Edition (MCC: CEA compression)", "mcc-custom"},
    };
    
    MapBuilder::MapBuilder(const MainWindow *main_window) : main_window(main_window) {
        auto *main_layout = new QHBoxLayout(this);
        this->setWindowTitle("Build a map");
        
        // Add options on the left
        {
            auto *options_widget = new QGroupBox("Parameters", this);
            auto *options_main_layout_widget = new QWidget(options_widget);
            auto *options_main_layout = new QGridLayout(options_main_layout_widget);
            auto *options_layout = new QVBoxLayout(options_widget);
            options_main_layout->setMargin(0);
            
            // Scenario path
            this->scenario_path = new QLineEdit();
            options_main_layout->addWidget(new QLabel("Scenario path:"), 0, 0);
            options_main_layout->addWidget(this->scenario_path, 0, 1);
            
            // Map type
            this->engine = new QComboBox();
            for(auto &i : build_type) {
                this->engine->addItem(i[0]);
            }
            options_main_layout->addWidget(new QLabel("Engine:"), 1, 0);
            options_main_layout->addWidget(this->engine, 1, 1);
            
            options_main_layout_widget->setLayout(options_main_layout);
            options_layout->addWidget(options_main_layout_widget);
            
            // Dummy widget (spacing)
            auto *dummy_widget = new QWidget();
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
            auto *console_widget = new QGroupBox("Console", this);
            auto *console_layout = new QVBoxLayout(console_widget);
            
            this->console_box = new ConsoleBox(this);
            console_layout->addWidget(this->console_box);
            
            console_widget->setLayout(console_layout);
            console_widget->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
            main_layout->addWidget(console_widget);
        }
    }
    
    void MapBuilder::compile_map() {
        // Process
        QProcess process;
        process.setProgram(this->main_window->executable_path("invader-build").string().c_str());
        
        // Set arguments
        QStringList arguments;
        for(auto &i : this->main_window->get_tags_directories()) {
            arguments << "--tags" << i.string().c_str();
        }
        
        arguments << "--game-engine" << build_type[this->engine->currentIndex()][1];
        arguments << this->scenario_path->text();
        
        process.setArguments(arguments);
        
        // Begin
        if(!process.startDetached()) {
            std::printf("Failed to start invader-edit-qt... -.-\n");
        }
    }
}
