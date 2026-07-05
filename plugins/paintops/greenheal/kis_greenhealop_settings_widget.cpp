/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_greenhealop_settings_widget.h"
#include "kis_greenhealop_settings.h"

#include <kis_properties_configuration.h>
#include <KisStandardOptionData.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <KisCompositeOpOptionWidget.h>
#include <KisSizeOptionWidget.h>
#include <KisMirrorOptionWidget.h>

KisGreenHealOpSettingsWidget::KisGreenHealOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision, parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    setObjectName("green heal option widget");

    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisMirrorOptionWidget>());
}

KisGreenHealOpSettingsWidget::~KisGreenHealOpSettingsWidget()
{
}

KisPropertiesConfigurationSP KisGreenHealOpSettingsWidget::configuration() const
{
    KisGreenHealOpSettings *config = new KisGreenHealOpSettings(resourcesInterface());
    config->setProperty("paintop", "greenheal");
    writeConfiguration(config);
    return config;
}
