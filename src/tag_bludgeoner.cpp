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
    struct BludgeonCommand {
        const char *command;
        const char *arguments[32];
    };

    #define MAKE_REFACTOR_CMD(from, to) { \
        .command = "invader-refactor", \
        .arguments = { "--groups", from, to, "--mode", "no-move" } \
    }

    static const BludgeonCommand STEPS_CMDS[] = {
        [TagBludgeoner::Step::FixUpTags] = {
            .command = "invader-bludgeon",
            .arguments = { "--type", "everything", "--batch", "*" }
        },
        [TagBludgeoner::Step::CleanUpTags] = {
            .command = "invader-strip",
            .arguments = { "--batch", "*" }
        },
        [TagBludgeoner::Step::RefactorChicagoExtendedToGeneric] = MAKE_REFACTOR_CMD("shader_transparent_chicago_extended", "shader_transparent_generic"),
        [TagBludgeoner::Step::RefactorChicagoToGeneric] = MAKE_REFACTOR_CMD("shader_transparent_chicago", "shader_transparent_generic"),
        [TagBludgeoner::Step::RefactorChicagoExtendedToChicago] = MAKE_REFACTOR_CMD("shader_transparent_chicago_extended", "shader_transparent_chicago"),
        [TagBludgeoner::Step::RefactorGbxmodelToModel] = MAKE_REFACTOR_CMD("gbxmodel", "model"),
        [TagBludgeoner::Step::RefactorModelToGbxmodel] = MAKE_REFACTOR_CMD("model", "gbxmodel")
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

            // Add options here
            options_main_layout->addWidget(new QLabel("Fix up tags:", options_widget), 1, 0);
            options_main_layout->addWidget(this->fix_up_tags = new QCheckBox(options_widget), 1, 1);

            options_main_layout->addWidget(new QLabel("Clean up tags:", options_widget), 2, 0);
            options_main_layout->addWidget(this->clean_up_tags = new QCheckBox(options_widget), 2, 1);

            options_main_layout->addWidget(new QLabel("Optimize shader references:", options_widget), 3, 0);
            options_main_layout->addWidget(this->optimize_shader_references = new QCheckBox(options_widget), 3, 1);

            options_main_layout->addWidget(new QLabel("Refactor model references:", options_widget), 4, 0);
            options_main_layout->addWidget(this->refactor_model_references = new QComboBox(options_widget), 4, 1);
            this->refactor_model_references->addItem("Do nothing");
            this->refactor_model_references->addItem("Change model references to gbxmodel (base HEK tags need it)", static_cast<uint>(TagBludgeoner::Step::RefactorModelToGbxmodel));
            this->refactor_model_references->addItem("Change gbxmodel references to model (Xbox porting)", static_cast<uint>(TagBludgeoner::Step::RefactorGbxmodelToModel));

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
        if(state == QProcess::ProcessState::NotRunning) {
            this->next_in_queue();
        }
    }

    void TagBludgeoner::bludgeon_tags() {
        if(this->fix_up_tags->isChecked()) {
            this->steps.emplace_back(Step::FixUpTags);
        }

        if(this->optimize_shader_references->isChecked()) {
            this->steps.emplace_back(Step::RefactorChicagoExtendedToGeneric);
            this->steps.emplace_back(Step::RefactorChicagoToGeneric);
            this->steps.emplace_back(Step::RefactorChicagoExtendedToChicago);
        }

        auto model_refactor = this->refactor_model_references->currentData();
        if(model_refactor.isValid()) {
            this->steps.emplace_back(static_cast<Step>(model_refactor.toUInt()));
        }

        if(this->clean_up_tags->isChecked()) {
            this->steps.emplace_back(Step::CleanUpTags);
        }

        this->tags_dir = this->tags->currentText();

        // Set up our process
        this->bludgeon_button->setEnabled(false);
        this->reset_contents();
        this->next_in_queue();
    }

    void TagBludgeoner::next_in_queue() {
        // We have to delete the process every time we call this
        this->cleanup_process();

        // Check if we're done
        if(this->steps.empty()) {
            this->bludgeon_button->setEnabled(true);
            return;
        }

        auto step = this->steps[0];
        this->steps.pop_front();
        auto &more_arguments = STEPS_CMDS[step];

        // Set arguments
        QStringList arguments;
        arguments << "--tags" << this->tags_dir;
        for(auto &i : more_arguments.arguments) {
            if(i == nullptr) {
                break;
            }
            arguments << i;
        }

        // Invoke
        this->process = new QProcess(this);
        this->attach_to_process(this->process);
        connect(this->process, &QProcess::stateChanged, this, &TagBludgeoner::set_ready);
        this->process->setProgram(this->main_window->executable_path(more_arguments.command).string().c_str());
        this->process->setArguments(arguments);
        this->process->start();
    }

    void TagBludgeoner::reject() {
        if(this->process != nullptr) {
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
                this->cleanup_process();
            }
        }
        QDialog::reject();
    }

    void TagBludgeoner::cleanup_process() {
        if(this->process != nullptr) {
            this->process->waitForFinished(-1);
            this->process->deleteLater();
            this->process = nullptr;
        }
    }
}
