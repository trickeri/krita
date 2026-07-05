/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "greenheal.h"
#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoCompositeOpRegistry.h>

#include <brushengine/kis_paintop_registry.h>
#include "kis_simple_paintop_factory.h"
#include "kis_greenhealop.h"
#include "kis_greenhealop_settings.h"
#include "kis_greenhealop_settings_widget.h"

K_PLUGIN_FACTORY_WITH_JSON(GreenHealFactory, "kritagreenhealpaintop.json", registerPlugin<GreenHeal>();)

GreenHeal::GreenHeal(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    // Recolour in place: the dab result must REPLACE (copy) the destination
    // under the brush, not composite over it — that is what keeps alpha exact.
    QStringList whiteList;
    whiteList << COMPOSITE_COPY;

    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisGreenHealOp, KisGreenHealOpSettings, KisGreenHealOpSettingsWidget>(
               "greenheal",
               i18nc("type of a brush engine, shown in the list of brush engines", "Green Heal"),
               KisPaintOpFactory::categoryStable(),
               "krita-greenheal.png", QString(), whiteList, 18));
}

GreenHeal::~GreenHeal()
{
}

#include "greenheal.moc"
