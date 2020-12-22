// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_TAG_TREE_WIDGET_HPP
#define SIX_SHOOTER_TAG_TREE_WIDGET_HPP

#include <QTreeWidget>

namespace SixShooter {
    class TagTreeWidget : public QTreeWidget {
        Q_OBJECT
    public:
        TagTreeWidget(QWidget *parent = nullptr);
        void set_data(QStringList tags);
    };
}

#endif
