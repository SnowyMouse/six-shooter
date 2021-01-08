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
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>

#include "main_window.hpp"
#include "settings_editor.hpp"
#include "settings.hpp"

namespace SixShooter {
    class SettingsEditor::Finder : public QGroupBox {
    private:
        const char *prompt;
        
        void find_directory() {
            QFileDialog qfd;
            qfd.setOption(QFileDialog::Option::ShowDirsOnly, true);
            qfd.setFileMode(QFileDialog::FileMode::Directory);
            qfd.setWindowTitle(prompt);
            if(qfd.exec()) {
                this->path->setText(qfd.selectedFiles()[0]);
            }
        }
        
    public:
        Finder(const char *text, QWidget *parent, const char *prompt, const char *default_setting) : QGroupBox(text, parent), prompt(prompt) {
            QHBoxLayout *layout = new QHBoxLayout(this);
            
            this->setMinimumWidth(600);
            
            this->path = new QLineEdit(default_setting, this);
            layout->addWidget(this->path);
            
            // Find button
            auto *button = new QPushButton("Find...", this);
            connect(button, &QPushButton::clicked, this, &SettingsEditor::Finder::find_directory);
            layout->addWidget(button);
            
            this->setLayout(layout);
        }
        
        QLineEdit *path;
    };
    
    SettingsEditor::SettingsEditor(MainWindow *main_window, bool exit_on_failure) : main_window(main_window), exit_on_failure(exit_on_failure) {
        auto *main_layout = new QVBoxLayout(this);
        this->setWindowTitle("Edit settings - Six Shooter");
        
        this->invader = new Finder("Invader directory", this, "Please find the folder where Invader is located", main_window->get_invader_directory().string().c_str());
        main_layout->addWidget(this->invader);
        
        this->maps = new Finder("Maps directory", this, "Please find your maps folder", main_window->get_maps_directory().string().c_str());
        main_layout->addWidget(this->maps);
        
        auto *tags_box = new QGroupBox("Tags directories", this);
        auto *tags_box_layout = new QVBoxLayout(tags_box);
        this->tags = new QTableWidget(this);
        this->tags_paths = main_window->get_tags_directories();
        tags_box_layout->addWidget(this->tags);
        
        auto *tags_box_buttons = new QWidget(tags_box);
        auto *tags_box_buttons_layout = new QHBoxLayout(tags_box_buttons);
        
        auto *move_up_button = new QPushButton("Move up", tags_box_buttons);
        connect(move_up_button, &QPushButton::clicked, this, &SettingsEditor::move_up);
        tags_box_buttons_layout->addWidget(move_up_button);
        
        auto *move_down_button = new QPushButton("Move down", tags_box_buttons);
        connect(move_down_button, &QPushButton::clicked, this, &SettingsEditor::move_down);
        tags_box_buttons_layout->addWidget(move_down_button);
        
        auto *add_button = new QPushButton("Add...", tags_box_buttons);
        connect(add_button, &QPushButton::clicked, this, &SettingsEditor::add_path);
        tags_box_buttons_layout->addWidget(add_button);
        
        auto *delete_button = new QPushButton("Delete", tags_box_buttons);
        connect(delete_button, &QPushButton::clicked, this, &SettingsEditor::remove_path);
        tags_box_buttons_layout->addWidget(delete_button);
        
        tags_box_buttons->setLayout(tags_box_buttons_layout);
        tags_box_layout->addWidget(tags_box_buttons);
        this->refresh_tags_table();
        
        tags_box->setLayout(tags_box_layout);
        main_layout->addWidget(tags_box);
        
        auto *qdbb = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        main_layout->addWidget(qdbb);
        
        connect(qdbb, &QDialogButtonBox::accepted, this, &SettingsEditor::accept);
        connect(qdbb, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(this->tags, &QTableWidget::cellChanged, this, &SettingsEditor::modified_path);
        
        this->setLayout(main_layout);
    }
    
    void SettingsEditor::move_up() {
        auto path_count = this->tags_paths.size();
        int delete_this = this->tags->currentRow();
        if(delete_this > 0 && delete_this < path_count) {
            auto path = this->tags_paths[delete_this];
            
            this->tags_paths.erase(this->tags_paths.begin() + delete_this);
            this->tags_paths.insert(this->tags_paths.begin() + delete_this - 1, path);
            
            this->refresh_tags_table();
            this->tags->setCurrentCell(delete_this - 1, 0);
        }
    }
    
    void SettingsEditor::move_down() {
        auto path_count = this->tags_paths.size();
        int delete_this = this->tags->currentRow();
        
        if(delete_this >= 0 && delete_this + 1 < path_count) {
            auto path = this->tags_paths[delete_this];
            
            this->tags_paths.erase(this->tags_paths.begin() + delete_this);
            this->tags_paths.insert(this->tags_paths.begin() + delete_this + 1, path);
            
            this->refresh_tags_table();
            this->tags->setCurrentCell(delete_this + 1, 0);
        }
    }
    
    void SettingsEditor::remove_path() {
        auto path_count = this->tags_paths.size();
        int delete_this = this->tags->currentRow();
        
        if(delete_this >= 0 && delete_this < path_count) {
            this->tags_paths.erase(this->tags_paths.begin() + delete_this);
            this->refresh_tags_table();
        }
    }
    
    void SettingsEditor::add_path() {
        QFileDialog qfd;
        qfd.setWindowTitle("Find a tags directory");
        qfd.setFileMode(QFileDialog::FileMode::Directory);
        if(qfd.exec()) {
            this->tags_paths.emplace_back(qfd.selectedFiles()[0].toStdString());
            this->refresh_tags_table();
        }
    }
    
    void SettingsEditor::reject() {
        if(this->exit_on_failure) {
            std::exit(EXIT_FAILURE);
        }
        QDialog::reject();
    }
    
    void SettingsEditor::accept() {
        // Make sure the Invader path is correct
        auto invader_path = this->invader->path->text();
        if(!this->main_window->invader_path_is_valid(invader_path.toStdString())) {
            QMessageBox qmd;
            qmd.setWindowTitle("Path error");
            qmd.setText("The Invader path is invalid (missing invader programs).");
            qmd.setIcon(QMessageBox::Icon::Critical);
            qmd.exec();
            return;
        }
        
        // Map path too
        auto map_path = this->maps->path->text();
        if(!std::filesystem::is_directory(map_path.toStdString())) {
            QMessageBox qmd;
            qmd.setWindowTitle("Path error");
            qmd.setText("The maps path does not point to a valid directory.");
            qmd.setIcon(QMessageBox::Icon::Critical);
            qmd.exec();
            return;
        }
        
        if(this->tags_paths.size() == 0) {
            QMessageBox qmd;
            qmd.setWindowTitle("Path error");
            qmd.setText("No tags directories are set");
            qmd.setIcon(QMessageBox::Icon::Critical);
            qmd.exec();
            return;
        }
        
        QStringList tags_path;
        for(auto &i : this->tags_paths) {
            if(!std::filesystem::is_directory(i)) {
                QMessageBox qmd;
                qmd.setWindowTitle("Path error");
                qmd.setText(QString("Tags directory \"") + (i.string().c_str()) + "\" does not point to a valid directory");
                qmd.setIcon(QMessageBox::Icon::Critical);
                qmd.exec();
                return;
            }
            tags_path << i.string().c_str();
        }
        
        // Update settings
        SixShooterSettings settings;
        settings.setValue("invader_path", invader_path);
        settings.setValue("maps_path", map_path);
        settings.setValue("tags_directories", tags_path);
        
        QDialog::accept();
    }
    
    void SettingsEditor::refresh_tags_table() {
        this->tags->clear();
        this->tags->setColumnCount(1);
        this->tags->horizontalHeader()->setStretchLastSection(true);
        this->tags->horizontalHeader()->hide();
        this->tags->verticalHeader()->hide();
        this->tags->setRowCount(this->tags_paths.size());
        this->tags->setSelectionMode(QAbstractItemView::SingleSelection);
        int index = 0;
        for(auto &i : this->tags_paths) {
            auto *item = new QTableWidgetItem(i.string().c_str());
            this->tags->setItem(index++, 0, item);
        }
    }
    
    void SettingsEditor::modified_path(int row, int) {
        this->tags_paths[row] = this->tags->item(row, 0)->text().toStdString();
    }
}
