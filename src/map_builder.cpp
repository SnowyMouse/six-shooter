// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPlainTextEdit>

#include "console_box.hpp"
#include "map_builder.hpp"

namespace SixShooter {
    MapBuilder::MapBuilder(const MainWindow *main_window) : main_window(main_window) {
        auto *main_layout = new QHBoxLayout(this);
        
        // Add options on the left
        {
            auto *options_widget = new QGroupBox("Parameters", this);
            auto *options_layout = new QVBoxLayout(options_widget);
            
            options_widget->setLayout(options_layout);
            options_widget->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
            main_layout->addWidget(options_widget);
        }
        
        // Add a console on the right
        {
            auto *console_widget = new QGroupBox("Console", this);
            auto *console_layout = new QVBoxLayout(console_widget);
            
            this->console_box = new ConsoleBox(this);
            console_layout->addWidget(this->console_box);
            
            console_widget->setLayout(console_layout);
            main_layout->addWidget(console_widget);
        }
        
        this->setLayout(main_layout);
    }
}
