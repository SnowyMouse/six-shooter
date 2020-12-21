// SPDX-License-Identifier: GPL-3.0-only

#include <QStyle>
#include <QGuiApplication>
#include <QScreen>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <filesystem>

#include "main_window.hpp"

namespace SixShooter {
    MainWindow::MainWindow() {
        this->setWindowTitle("Six Shooter");
        
        // Get settings
        QSettings settings;
        
        // Do we have Invader present?
        if(settings.value("invader_path").isNull()) {
            if(!find_invader()) {
                std::exit(EXIT_FAILURE);
            }
        }
        else {
            if(!invader_path_is_valid(settings.value("invader_path").toString().toStdString())) {
                QMessageBox qmd;
                qmd.setWindowTitle("Path error");
                qmd.setText("The Invader path stored in the application config is no longer valid. You will have to re-find it.");
                qmd.setIcon(QMessageBox::Icon::Warning);
                qmd.exec();
                if(!find_invader()) {
                    std::exit(EXIT_FAILURE);
                }
            }
        }
        this->invader_path = settings.value("invader_path").toString().toStdString();
        
        // Set up the GUI
        QWidget *widget = new QWidget(this);
        this->setCentralWidget(widget);
        
        // Center the window
        this->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                this->rect().size(),
                QGuiApplication::primaryScreen()->geometry()
            )
        );
    }
    
    bool MainWindow::find_invader() {
        while(true) {
            // Initialize the file dialog
            QFileDialog qfd;
            qfd.setOptions(QFileDialog::Option::ReadOnly | QFileDialog::Option::ShowDirsOnly);
            qfd.setWindowTitle("Find the folder where Invader is located");
            if(qfd.exec()) {
                // Look for it!
                auto path = qfd.selectedFiles()[0];
                if(invader_path_is_valid(path.toStdString())) {
                    QSettings().setValue("invader_path", path);
                    return true;
                }
                else {
                    QMessageBox qmd;
                    qmd.setWindowTitle("Path error");
                    qmd.setText("The path (" + path + ") is not valid.");
                    qmd.setIcon(QMessageBox::Icon::Critical);
                    qmd.exec();
                    continue;
                }
            }
            else {
                return false;
            }
        }
    }
    
    bool MainWindow::invader_path_is_valid(const std::filesystem::path &path) {
        auto executable_exists = [&path](const char *executable) -> bool {
            const char *executable_extension = "";
            
            #ifdef _WIN32
            executable_extension = ".exe";
            #endif
            
            return std::filesystem::exists(path / (std::string(executable) + executable_extension));
        };
        
        // Check if these exist
        return executable_exists("invader-build") && 
               executable_exists("invader-extract") &&
               executable_exists("invader-info");
    }
}
