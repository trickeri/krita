/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GREENHEALOP_SETTINGS_H_
#define KIS_GREENHEALOP_SETTINGS_H_

#include <kis_brush_based_paintop_settings.h>
#include <kis_types.h>

class KisGreenHealOpSettings : public KisBrushBasedPaintOpSettings
{

public:
    KisGreenHealOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisGreenHealOpSettings() override;

    // Heal in place: always paint onto the existing layer data so repeated
    // passes accumulate toward zero residual green.
    bool paintIncremental() override;

    bool hasPatternSettings() const override;
};

#endif // KIS_GREENHEALOP_SETTINGS_H_
