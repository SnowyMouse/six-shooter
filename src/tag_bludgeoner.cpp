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
#include "tag_bludgeoner.hpp"

namespace SixShooter {
    struct BludgeonLevel {
        const char *name;
        std::size_t arguments_count = 0;
        const char *arguments[32];
    };
    
    static const BludgeonLevel levels[] = {
        {
            "Basic (fixes common issues with stock maps)",
            6,
            { "-T", "invalid-indices", "-T", "invalid-enums", "-T", "out-of-range" }
        },
        {
            "Extra (fixes common issues with tags made with other tools)",
            18,
            { "-T", "invalid-reference-classes", "-T", "excessive-script-nodes", "-T", "missing-script-source", "-T", "invalid-model-markers", "-T", "nonnormal-vectors", "-T", "invalid-strings", "-T", "invalid-indices", "-T", "invalid-enums", "-T", "out-of-range" }
        },
        {
            "Sounds (fixes issues with some sounds - SLOW)",
            2,
            { "-T", "incorrect-sound-buffer" }
        },
        {
            "Exodia (do everything - dangerous)",
            1,
            { "--all" }
        }
    };
    
    TagBludgeoner::TagBludgeoner(const MainWindow *main_window) : main_window(main_window) {
        auto *main_layout = new QHBoxLayout(this);
        this->setWindowTitle("Tag bludgeoner - Six Shooter");
        
        // Add options on the left
        {
            auto *options_widget = new QGroupBox("Parameters", this);
            auto *options_main_layout_widget = new QWidget(options_widget);
            auto *options_main_layout = new QGridLayout(options_main_layout_widget);
            auto *options_layout = new QVBoxLayout(options_widget);
            options_main_layout->setMargin(0);
            
            // Add item
            this->tags = new QComboBox(options_widget);
            for(auto &i : this->main_window->get_tags_directories()) {
                this->tags->insertItem(0, i.string().c_str());
            }
            this->tags->setMinimumWidth(400);
            this->tags->setMaximumWidth(400);
            options_main_layout->addWidget(new QLabel("Tags directory:"), 0, 0);
            options_main_layout->addWidget(this->tags, 0, 1);
            
            // Add level
            this->level = new QComboBox(options_widget);
            for(auto &i : levels) {
                this->level->addItem(i.name);
            }
            options_main_layout->addWidget(new QLabel("Level:"), 1, 0);
            options_main_layout->addWidget(this->level, 1, 1);
            
            options_layout->addWidget(options_main_layout_widget);
            
            // Dummy widget (spacing)
            auto *dummy_widget = new QWidget(options_widget);
            dummy_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
            options_layout->addWidget(dummy_widget);
            
            // Build button
            auto *compile_button = new QPushButton("Bludgeon", options_widget);
            connect(compile_button, &QPushButton::clicked, this, &TagBludgeoner::bludgeon_tags);
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
    
    TagBludgeoner::~TagBludgeoner() {
        if(this->process != nullptr) {
            this->process->kill();
        }
    }
    
    void TagBludgeoner::bludgeon_tags() {
        if(this->process != nullptr) {
            this->process->kill();
            delete this->process;
            this->process = nullptr;
        }
        
        // Process
        this->process = new QProcess(this);
        this->process->setProgram(this->main_window->executable_path("invader-bludgeon").string().c_str());
        
        // Set arguments
        QStringList arguments;
        arguments << "--tags" << this->tags->currentText();
        arguments << "--all";
        auto &more_arguments = levels[this->level->currentIndex()];
        for(std::size_t i = 0; i < more_arguments.arguments_count; i++) {
            arguments << more_arguments.arguments[i];
        }
        
        // Invoke
        this->process->setArguments(arguments);
        this->console_box_stdout->attach_to_process(this->process, ConsoleBox::StandardOutput);
        this->console_box_stderr->attach_to_process(this->process, ConsoleBox::StandardError);
        this->process->start();
    }
}
