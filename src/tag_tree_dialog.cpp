// SPDX-License-Identifier: GPL-3.0-only

#include <QVBoxLayout>

#include "tag_tree_dialog.hpp"
#include "tag_tree_widget.hpp"

namespace SixShooter {
    TagTreeDialog::TagTreeDialog(QWidget *parent) : QDialog(parent) {
        this->tree = new TagTreeWidget(this);
        auto *layout = new QVBoxLayout(this);
        connect(this->tree, &TagTreeWidget::itemDoubleClicked, this, &TagTreeDialog::double_clicked);
        layout->addWidget(this->tree);
        this->setLayout(layout);
        this->setMinimumHeight(600);
        this->setMinimumWidth(600);
    }
    
    void TagTreeDialog::set_data(QStringList tags) {
        this->tree->set_data(tags);
    }
    
    void TagTreeDialog::double_clicked(QTreeWidgetItem *item, int column) {
        auto data = item->data(column, Qt::UserRole);
        if(!data.isNull()) {
            this->result = data.toString();
            QDialog::accept();
        }
    }
    
    QString TagTreeDialog::get_result() {
        return this->result;
    }
}
