// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QIcon>

#ifdef SIX_SHOOTER_WIN32_EXE_STATIC_LINK
#include <QtPlugin>
#endif

#include "main_window.hpp"

#ifdef SIX_SHOOTER_WIN32_EXE_STATIC_LINK
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#endif

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    a.setOrganizationName("SnowyMouse");
    a.setWindowIcon(QIcon(":icon/six-shooter.ico"));
    
    SixShooter::MainWindow w;
    w.show();
    
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
