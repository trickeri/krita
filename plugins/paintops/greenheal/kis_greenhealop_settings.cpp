/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_greenhealop_settings.h"

KisGreenHealOpSettings::KisGreenHealOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface)
{
}

KisGreenHealOpSettings::~KisGreenHealOpSettings()
{
}

bool KisGreenHealOpSettings::paintIncremental()
{
    return true; // We always paint on the existing data
}

bool KisGreenHealOpSettings::hasPatternSettings() const
{
    return false;
}
