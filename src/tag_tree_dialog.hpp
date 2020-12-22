// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_TAG_TREE_DIALOG_HPP
#define SIX_SHOOTER_TAG_TREE_DIALOG_HPP

#include <QDialog>

class QTreeWidgetItem;

namespace SixShooter {
    class TagTreeWidget;
    
    class TagTreeDialog : public QDialog {
        Q_OBJECT
    public:
        TagTreeDialog(QWidget *parent = nullptr);
        void set_data(QStringList tags);
        QString get_result();
        
    private:
        TagTreeWidget *tree;
        QString result;
        void double_clicked(QTreeWidgetItem *item, int column);
    };
}

#endif
