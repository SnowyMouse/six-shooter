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

#include "tag_tree_widget.hpp"

namespace SixShooter {
    TagTreeWidget::TagTreeWidget(QWidget *parent) : QTreeWidget(parent) {
        this->header()->setStretchLastSection(true);
        this->setColumnCount(1);
        this->setHeaderHidden(true);
        this->setAlternatingRowColors(true);
        this->setAnimated(false);
    }
    
    void TagTreeWidget::set_data(QStringList tags) {
        QIcon dir_icon = QFileIconProvider().icon(QFileIconProvider::Folder);
        QIcon file_icon = QFileIconProvider().icon(QFileIconProvider::File);
        
        for(auto &tag : tags) {
            if(tag.endsWith(".none")) {
                continue;
            }
            
            QString formatted = tag.replace("/", "\\");
            auto dir_split = formatted.split("\\");
            auto dir_split_length = dir_split.size();
            
            if(dir_split_length == 0) {
                continue; // ok weird
            }
            
            QTreeWidgetItem *directory = nullptr;
            for(std::size_t i = 0; i < dir_split_length; i++) {
                // Adding it now!
                if(i + 1 == dir_split_length) {
                    auto *item = new QTreeWidgetItem(QStringList(dir_split[i]));
                    item->setData(0, Qt::UserRole, tag);
                    item->setIcon(0, file_icon);
                    
                    if(directory == nullptr) {
                        int child_count = this->topLevelItemCount();
                        int insertion_point = child_count;
                        
                        for(int j = 0; j < child_count; j++) {
                            auto *item = this->topLevelItem(j);
                            if(item->text(0) > dir_split[i] && item->icon(0).name() != dir_icon.name()) {
                                insertion_point = j;
                                break;
                            }
                        }
                        
                        this->insertTopLevelItem(insertion_point, item);
                    }
                    else {
                        int child_count = directory->childCount();
                        int insertion_point = child_count;
                        
                        for(int j = 0; j < child_count; j++) {
                            auto *item = directory->child(j);
                            if(item->text(0) > dir_split[i] && item->icon(0).name() != dir_icon.name()) {
                                insertion_point = j;
                                break;
                            }
                        }
                        
                        directory->insertChild(insertion_point, item);
                    }
                }
                
                // Find the directory with it
                else {
                    auto directory_we_are_looking_for = [&directory, &dir_icon, &i, &dir_split](QTreeWidgetItem *item) -> bool {
                        if(item->text(0) == dir_split[i] && item->icon(0).name() == dir_icon.name()) {
                            directory = item;
                            return true;
                        }
                        else {
                            return false;
                        }
                    };
                    
                    // Hold this in case we change it
                    auto *old_directory = directory;
                    
                    if(directory == nullptr) {
                        int child_count = this->topLevelItemCount();
                        for(int i = 0; i < child_count; i++) {
                            if(directory_we_are_looking_for(this->topLevelItem(i))) {
                                break;
                            }
                        }
                    }
                    else {
                        int child_count = directory->childCount();
                        for(int i = 0; i < child_count; i++) {
                            if(directory_we_are_looking_for(directory->child(i))) {
                                break;
                            }
                        }
                    }
                    
                    // Directory changed. Proceed.
                    if(old_directory != directory) {
                        continue;
                    }
                    
                    // Not found? Make the directory then.
                    auto *new_directory = new QTreeWidgetItem(QStringList(dir_split[i]));
                    new_directory->setIcon(0, dir_icon);
                    if(directory == nullptr) {
                        int child_count = this->topLevelItemCount();
                        int insertion_point = child_count;
                        
                        for(int j = 0; j < child_count; j++) {
                            auto *item = this->topLevelItem(j);
                            if(item->text(0) > dir_split[i] || item->icon(0).name() != dir_icon.name()) {
                                insertion_point = j;
                                break;
                            }
                        }
                        
                        this->insertTopLevelItem(insertion_point, new_directory);
                    }
                    else {
                        int child_count = directory->childCount();
                        int insertion_point = child_count;
                        
                        for(int j = 0; j < child_count; j++) {
                            auto *item = directory->child(j);
                            if(item->text(0) > dir_split[i] || item->icon(0).name() != dir_icon.name()) {
                                insertion_point = j;
                                break;
                            }
                        }
                        
                        directory->insertChild(insertion_point, new_directory);
                    }
                    directory = new_directory;
                }
            }
        }
    }
}
