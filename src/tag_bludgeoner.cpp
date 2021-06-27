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
#include <QMessageBox>
#include <QFileIconProvider>

#include "console_box.hpp"
#include "main_window.hpp"
#include "tag_bludgeoner.hpp"

namespace SixShooter {
    struct BludgeonLevel {
        const char *name;
        std::size_t arguments_count = 0;
        const char *command;
        const char *arguments[32];
    };
    
    static const BludgeonLevel levels[] = {
        {
            "Basic (fixes common issues)",
            19,
            "invader-bludgeon",
            { "--type", "invalid-reference-classes", "--type", "excessive-script-nodes", "--type", "missing-script-source", "--type", "invalid-model-markers", "--type", "nonnormal-vectors", "--type", "invalid-strings", "--type", "invalid-indices", "--type", "invalid-enums", "--type", "out-of-range", "--all" }
        },
        {
            "Sounds (fixes issues with some sounds - slow if on a toaster)",
            3,
            "invader-bludgeon",
            { "--type", "incorrect-sound-buffer", "--all" }
        },
        {
            "Change model references to gbxmodel (base HEK tags need it)",
            5,
            "invader-refactor",
            { "--mode", "no-move", "--class", "model", "gbxmodel" }
        },
        {
            "Change gbxmodel references to model (Xbox porting)",
            5,
            "invader-refactor",
            { "--mode", "no-move", "--class", "gbxmodel", "model" }
        },
        {
            "Clean up (strip unused data - useful if modding Halo: CEA with official tools)",
            1,
            "invader-strip",
            { "--all" }
        },
        {
            "Exodia (bludgeon everything - slow if on a toaster)",
            3,
            "invader-bludgeon",
            { "--type", "everything", "--all" }
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
            options_main_layout->setContentsMargins(0, 0, 0, 0);
            
            // Add item
            this->tags = new QComboBox(options_widget);
            for(auto &i : this->main_window->get_tags_directories()) {
                this->tags->insertItem(0, i.string().c_str());
            }
            auto *tags_directory_label = new QLabel("Tags directory:", options_widget);
            tags_directory_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            options_main_layout->addWidget(tags_directory_label, 0, 0);
            options_main_layout->addWidget(this->tags, 0, 1);
            
            // Add level
            this->level = new QComboBox(options_widget);
            for(auto &i : levels) {
                this->level->addItem(i.name);
            }
            auto *level_label = new QLabel("Level:", options_widget);
            level_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            options_main_layout->addWidget(level_label, 1, 0);
            options_main_layout->addWidget(this->level, 1, 1);
            
            options_layout->addWidget(options_main_layout_widget);
            
            // Dummy widget (spacing)
            auto *dummy_widget = new QWidget(options_widget);
            dummy_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
            options_layout->addWidget(dummy_widget);
            
            // Bludgeon button
            this->bludgeon_button = new QPushButton("Bludgeon", options_widget);
            connect(this->bludgeon_button, &QPushButton::clicked, this, &TagBludgeoner::bludgeon_tags);
            options_layout->addWidget(this->bludgeon_button);
            
            // Set the layout
            options_widget->setLayout(options_layout);
            
            options_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
            main_layout->addWidget(options_widget);
        }
        
        // Add a console on the right
        {
            main_layout->addWidget(this->get_console_widget());
        }
    }
    
    void TagBludgeoner::set_ready(QProcess::ProcessState state) {
        this->bludgeon_button->setEnabled(state == QProcess::ProcessState::NotRunning);
    }
    
    void TagBludgeoner::bludgeon_tags() {
        if(this->process != nullptr) {
            this->process->waitForFinished(-1);
            delete this->process;
            this->process = nullptr;
        }
        
        // Process
        auto &more_arguments = levels[this->level->currentIndex()];
        this->process = new QProcess(this);
        connect(this->process, &QProcess::stateChanged, this, &TagBludgeoner::set_ready);
        this->process->setProgram(this->main_window->executable_path(more_arguments.command).string().c_str());
        
        // Set arguments
        QStringList arguments;
        arguments << "--tags" << this->tags->currentText();
        for(std::size_t i = 0; i < more_arguments.arguments_count; i++) {
            arguments << more_arguments.arguments[i];
        }
        
        // Invoke
        this->process->setArguments(arguments);
        this->attach_to_process(this->process);
        this->process->start();
    }
    
    void TagBludgeoner::reject() {
        if(this->process) {
            if(this->process->state() == QProcess::ProcessState::Running) {
                QMessageBox qmb;
                qmb.setWindowTitle("Tag bludgeoning in progress");
                qmb.setText("Are you sure you want to stop bludgeoning tags?\n\nAborting the bludgeon process may leave your tags directory in an inconsistent or potentially corrupted state.");
                qmb.setStandardButtons(QMessageBox::StandardButton::Abort | QMessageBox::StandardButton::Cancel);
                qmb.setIcon(QMessageBox::Icon::Warning);
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
