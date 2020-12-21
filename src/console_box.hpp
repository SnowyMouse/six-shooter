// SPDX-License-Identifier: GPL-3.0-only

#include <QTextEdit>

namespace SixShooter {
    class ConsoleBox : public QTextEdit {
        Q_OBJECT
    public:
        ConsoleBox(QWidget *parent = nullptr);
    };
}
