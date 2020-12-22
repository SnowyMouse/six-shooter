// SPDX-License-Identifier: GPL-3.0-only

#include <QDialog>
#include <filesystem>
#include <vector>

class QLineEdit;
class QComboBox;
class QProcess;

namespace SixShooter {
    class MainWindow;
    class ConsoleBox;
    
    class MapBuilder : public QDialog {
        Q_OBJECT
        friend class MainWindow;
    private:
        MapBuilder(const MainWindow *main_window);
        const MainWindow *main_window;
        ConsoleBox *console_box;
        QLineEdit *scenario_path;
        QComboBox *engine;
        QProcess *process = nullptr;
        
        void compile_map();
    };
}
