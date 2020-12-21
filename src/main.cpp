// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include <QStyle>
#include <QScreen>
#include "main_window.hpp"

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    a.setOrganizationName("SnowyMouse");
    
    SixShooter::MainWindow w;
    w.show();// Center the window
    
    w.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            w.size(),
            QGuiApplication::primaryScreen()->geometry()
        )
    );
    
    w.setFixedSize(w.size());
    
    return a.exec();
}
