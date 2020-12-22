// SPDX-License-Identifier: GPL-3.0-only

#include <QTreeWidget>

namespace SixShooter {
    class TagTreeWidget : public QTreeWidget {
        Q_OBJECT
    public:
        TagTreeWidget(QWidget *parent = nullptr);
        void set_data(QStringList tags);
        
    private:
        
    };
}
