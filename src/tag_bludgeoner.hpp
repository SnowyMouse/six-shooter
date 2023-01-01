// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_TAG_BLUDGEONER_HPP
#define SIX_SHOOTER_TAG_BLUDGEONER_HPP

#include <QDialog>
#include <QProcess>
#include <string>
#include <vector>
#include <filesystem>
#include <deque>

#include "console_dialog.hpp"

class QComboBox;
class QPushButton;
class QCheckBox;

namespace SixShooter {
    class MainWindow;

    class TagBludgeoner : public ConsoleDialog {
        Q_OBJECT
        friend class MainWindow;

    public:
        enum Step {
            FixUpTags,
            CleanUpTags,
            RefactorChicagoExtendedToGeneric,
            RefactorChicagoToGeneric,
            RefactorChicagoExtendedToChicago,
            RefactorGbxmodelToModel,
            RefactorModelToGbxmodel,
        };

    private:
        std::deque<Step> steps;

        TagBludgeoner(const MainWindow *main_window);
        const MainWindow *main_window;

        QString tags_dir;

        QComboBox *tags;
        QComboBox *refactor_model_references;
        QPushButton *bludgeon_button;
        QProcess *process = nullptr;

        QCheckBox *fix_up_tags;
        QCheckBox *clean_up_tags;
        QCheckBox *optimize_shader_references;

        void reject() override;

        void set_ready(QProcess::ProcessState);
        void bludgeon_tags();

        void next_in_queue();
        void cleanup_process();
    };
}

#endif
