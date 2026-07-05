/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GREENHEALOP_SETTINGS_WIDGET_H_
#define KIS_GREENHEALOP_SETTINGS_WIDGET_H_

#include <kis_brush_based_paintop_options_widget.h>
#include <kis_image.h>

class KisGreenHealOpSettingsWidget : public KisBrushBasedPaintopOptionWidget
{

    Q_OBJECT

public:

    KisGreenHealOpSettingsWidget(QWidget* parent = 0);

    ~KisGreenHealOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
};

#endif // KIS_GREENHEALOP_SETTINGS_WIDGET_H_
