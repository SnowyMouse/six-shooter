// SPDX-License-Identifier: GPL-3.0-only

#ifndef SIX_SHOOTER_SETTINGS_HPP
#define SIX_SHOOTER_SETTINGS_HPP

#include <QSettings>

namespace SixShooter {
    class SixShooterSettings : public QSettings {
    public:
        SixShooterSettings();
    };
}

#endif
