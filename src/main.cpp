// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include "main_window.hpp"

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    a.setOrganizationName("SnowyMouse");
    
    SixShooter::MainWindow w;
    w.show();
    
    return a.exec();
}
