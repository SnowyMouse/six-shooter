// SPDX-License-Identifier: GPL-3.0-only

#include "settings.hpp"

namespace SixShooter {
    SixShooterSettings::SixShooterSettings() :
        #ifdef _WIN32
        QSettings("six-shooter.ini", QSettings::IniFormat)
        #else
        QSettings()
        #endif
    {};
}
