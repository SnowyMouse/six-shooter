// SPDX-License-Identifier: GPL-3.0-only

#include <QGroupBox>
#include <QVBoxLayout>

#include "console_dialog.hpp"
#include "console_box.hpp"

namespace SixShooter {
    ConsoleDialog::ConsoleDialog() {
        this->console_widget = new QWidget(this);
        auto *console_layout = new QVBoxLayout(console_widget);
        console_layout->setContentsMargins(0, 0, 0, 0);
        
        auto *stdout_widget = new QGroupBox("Output", this);
        auto *stdout_layout = new QVBoxLayout(stdout_widget);
        this->stdout_box = new ConsoleBox(console_widget);
        stdout_layout->addWidget(this->stdout_box);
        stdout_widget->setLayout(stdout_layout);
        console_layout->addWidget(stdout_widget);
        
        auto *stderr_widget = new QGroupBox("Errors", this);
        auto *stderr_layout = new QVBoxLayout(stderr_widget);
        this->stderr_box = new ConsoleBox(console_widget);
        stderr_layout->addWidget(this->stderr_box);
        stderr_widget->setLayout(stderr_layout);
        console_layout->addWidget(stderr_widget);
        
        console_widget->setLayout(console_layout);
        console_widget->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
    }
    
    ConsoleDialog::~ConsoleDialog() {
        
    }
    
    void ConsoleDialog::attach_to_process(QProcess *process) {
        this->stdout_box->attach_to_process(process, ConsoleBox::StandardOutput);
        this->stderr_box->attach_to_process(process, ConsoleBox::StandardError);
    }
    
    QWidget *ConsoleDialog::get_console_widget() {
        return this->console_widget;
    }
}
