// SPDX-License-Identifier: GPL-3.0-only

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QProcess>
#include <QLineEdit>
#include <QTreeWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QScreen>
#include <QFileDialog>
#include <QGuiApplication>

#include "console_box.hpp"
#include "main_window.hpp"
#include "map_extractor.hpp"
#include "tag_tree_widget.hpp"

namespace SixShooter {
    MapExtractor::MapExtractor(const MainWindow *main_window, const std::filesystem::path &path) : main_window(main_window), path(path) {
        auto *main_layout = new QHBoxLayout(this);
        this->setWindowTitle(QString(path.string().c_str()) + " - Extract a tag - Six Shooter");

        auto *left_widget = new QWidget(this);
        auto *left_layout = new QVBoxLayout(left_widget);
        left_layout->setContentsMargins(0, 0, 0, 0);

        // Add options on the left
        {
            auto *options_widget = new QGroupBox("Parameters", left_widget);
            auto *options_layout = new QGridLayout(options_widget);

            // Add item
            this->tags = new QComboBox(options_widget);
            for(auto &i : this->main_window->get_tags_directories()) {
                this->tags->insertItem(0, i.string().c_str());
            }

            auto *tags_label = new QLabel("Tags directory:", options_widget);
            tags_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            options_layout->addWidget(tags_label, 0, 0);
            options_layout->addWidget(this->tags, 0, 1);

            this->non_mp_globals = new QCheckBox(options_widget);
            auto *non_mp_globals_label = new QLabel("Extract non-multiplayer globals:", options_widget);
            non_mp_globals_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            options_layout->addWidget(non_mp_globals_label, 1, 0);
            options_layout->addWidget(this->non_mp_globals, 1, 1);

            this->overwrite = new QCheckBox(options_widget);
            auto *overwrite_label = new QLabel("Overwrite:", options_widget);
            overwrite_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            options_layout->addWidget(overwrite_label, 2, 0);
            options_layout->addWidget(this->overwrite, 2, 1);

            this->ignore_resources = new QCheckBox(options_widget);
            auto *ignore_external_label = new QLabel("Ignore external tags:", options_widget);
            ignore_external_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            options_layout->addWidget(ignore_external_label, 3, 0);
            options_layout->addWidget(this->ignore_resources, 3, 1);

            this->use_maps_preferences = new QCheckBox(options_widget);
            auto *use_maps_preferences_label = new QLabel("Use maps folder from preferences:", options_widget);
            use_maps_preferences_label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            options_layout->addWidget(use_maps_preferences_label, 4, 0);
            options_layout->addWidget(this->use_maps_preferences, 4, 1);

            auto *generate_index_button = new QPushButton("Generate index file", options_widget);
            connect(generate_index_button, &QPushButton::clicked, this, &MapExtractor::generate_index_file);
            options_layout->addWidget(generate_index_button, 5, 1);

            // Set the layout
            options_widget->setLayout(options_layout);

            options_widget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
            left_layout->addWidget(options_widget);
        }

        // Tags
        {
            auto *tags_widget = new QGroupBox("Tags", left_widget);
            auto *tags_layout = new QVBoxLayout(tags_widget);

            this->map_tags = new TagTreeWidget(tags_widget);
            connect(this->map_tags, &TagTreeWidget::itemDoubleClicked, this, &MapExtractor::double_clicked);
            tags_layout->addWidget(this->map_tags);

            // Extract button
            this->extract_button = new QPushButton("Extract all tags", tags_widget);
            connect(this->extract_button, &QPushButton::clicked, this, &MapExtractor::extract_full_map);
            tags_layout->addWidget(this->extract_button);

            tags_widget->setLayout(tags_layout);
            left_layout->addWidget(tags_widget);
        }

        left_widget->setLayout(left_layout);
        main_layout->addWidget(left_widget);

        // Add a console on the right
        main_layout->addWidget(this->get_console_widget());

        // Set this
        auto screen_geometry = QGuiApplication::primaryScreen()->geometry();
        this->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                QSize(screen_geometry.width() / 5 * 4, screen_geometry.height() / 5 * 4),
                screen_geometry
            )
        );

        this->reload_info();
    }

    void MapExtractor::set_ready(QProcess::ProcessState state) {
        this->extract_button->setEnabled(state == QProcess::ProcessState::NotRunning);
        this->map_tags->setEnabled(state == QProcess::ProcessState::NotRunning);
    }

    void MapExtractor::extract_map(const std::vector<std::string> &filter, bool recursive, bool overwrite_anyway) {
        if(this->process != nullptr) {
            this->process->kill();
            delete this->process;
            this->process = nullptr;
        }

        // Process
        this->process = new QProcess(this);
        connect(this->process, &QProcess::stateChanged, this, &MapExtractor::set_ready);
        this->process->setProgram(this->main_window->executable_path("invader-extract").string().c_str());

        // Set arguments
        QStringList arguments;
        arguments << "--tags" << this->tags->currentText();

        if(this->use_maps_preferences->isChecked()) {
            arguments << "--maps" << this->main_window->get_maps_directory().string().c_str();
        }

        arguments << this->path.string().c_str();

        if(this->non_mp_globals->isChecked()) {
            arguments << "--non-mp-globals";
        }

        if(recursive) {
            arguments << "--recursive";
        }

        if(this->overwrite->isChecked() || overwrite_anyway) {
            arguments << "--overwrite";
        }

        if(this->ignore_resources->isChecked()) {
            arguments << "--ignore-resources";
        }

        for(auto &i : filter) {
            arguments << "--search" << i.c_str();
        }

        // Invoke
        this->process->setArguments(arguments);
        this->attach_to_process(this->process);
        this->reset_contents();
        this->process->start();
    }

    void MapExtractor::extract_full_map() {
        return extract_map();
    }

    QString MapExtractor::get_map_info(const char *what) const {
        // Process
        QProcess process;
        process.setProgram(this->main_window->executable_path("invader-info").string().c_str());

        QStringList arguments;
        arguments << "--type" << what;
        arguments << this->path.string().c_str();
        process.setArguments(arguments);

        process.start();
        process.waitForFinished(-1);

        return QString(process.readAllStandardOutput()).replace("\r", "").trimmed();
    }

    void MapExtractor::reload_info() {
        QProcess process;
        process.setProgram(this->main_window->executable_path("invader-info").string().c_str());
        this->attach_to_process(&process);
        process.setArguments(QStringList(this->path.string().c_str()));
        process.start();
        process.waitForFinished(-1);

        auto tags = get_map_info("tags").split("\n");
        this->map_tags->set_data(tags);
    }

    void MapExtractor::double_clicked(QTreeWidgetItem *item, int column) {
        auto data = item->data(column, Qt::UserRole);
        if(!data.isNull()) {
            QMessageBox options;
            options.setWindowTitle("Extraction options");
            options.setIcon(QMessageBox::Icon::Question);
            options.setText(QString("You are about to extract ") + data.toString());
            options.setStandardButtons(QMessageBox::StandardButton::Cancel);
            options.addButton("Extract (single tag)", QMessageBox::ButtonRole::AcceptRole);
            options.addButton("Extract (recursive)", QMessageBox::ButtonRole::AcceptRole);

            auto *overwrite = new QCheckBox("Overwrite tag(s) on disk (if present)", &options);
            options.setCheckBox(overwrite);

            int r = options.exec();

            if(options.result() == QMessageBox::StandardButton::Cancel) {
                return;
            }

            std::vector<std::string> filters;
            filters.emplace_back(data.toString().toStdString());
            this->extract_map(filters, r == 1, overwrite->isChecked());
        }
    }

    void MapExtractor::generate_index_file() {
        QFileDialog qfd;
        qfd.setFileMode(QFileDialog::FileMode::AnyFile);
        qfd.setWindowTitle("Save the index file");
        qfd.setNameFilter("Text file (*.txt)");
        qfd.selectFile(QString(this->path.stem().string().c_str()) + ".txt");
        qfd.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
        if(qfd.exec()) {
            auto path = qfd.selectedFiles()[0];

            // Wait until the process finished
            if(this->process) {
                this->process->waitForFinished(-1);
                this->process = nullptr;
                delete this->process;
            }

            // Spawn a new one
            QProcess process(this);
            process.setProgram(this->main_window->executable_path("invader-index").string().c_str());
            this->attach_to_process(&process);
            QStringList arguments;
            arguments << this->path.string().c_str() << path;
            process.setArguments(arguments);
            process.start();
            process.waitForFinished(-1);
        }
    }

    void MapExtractor::reject() {
        if(this->process) {
            if(this->process->state() == QProcess::ProcessState::Running) {
                QMessageBox qmb;
                qmb.setWindowTitle("Tag extraction in progress");
                qmb.setText("Are you sure you want to stop extracting tags?\n\nAborting the extraction process may leave your tags directory in an inconsistent or potentially corrupted state.");
                qmb.setIcon(QMessageBox::Icon::Warning);
                qmb.setStandardButtons(QMessageBox::StandardButton::Abort | QMessageBox::StandardButton::Cancel);
                if(qmb.exec() == QMessageBox::StandardButton::Cancel) {
                    return;
                }
                this->process->kill();
                this->process->waitForFinished(-1);
            }
            delete this->process;
            this->process = nullptr;
        }
        QDialog::reject();
    }
}
