/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_greenhealop.h"

#include <cmath>

#include <QColor>
#include <QRect>

#include <kis_debug.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_fixed_paint_device.h>
#include <kis_sequential_iterator.h>
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>
#include <kis_paintop_settings.h>
#include "kis_greenhealop_settings.h"

namespace {
// Spill (G - max(R,B), in 0..255) at or above which a pixel is recoloured all
// the way to the sampled colour. Below it the recolour scales down linearly, so
// only-slightly-green pixels are nudged, not repainted. Tunable knob candidate.
const qreal GREENHEAL_FULL_RECOLOUR_SPILL = 24.0;
}

KisGreenHealOp::KisGreenHealOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_sizeOption(settings.data())
    , m_rotationOption(settings.data())
{
    Q_UNUSED(node);
    Q_UNUSED(image);
    Q_ASSERT(settings);
    Q_ASSERT(painter);

    m_tmpDevice = source()->createCompositionSourceDevice();
    m_rotationOption.applyFanCornersInfo(this);
}

KisGreenHealOp::~KisGreenHealOp()
{
}

void KisGreenHealOp::applyGreenHeal(KisPaintDeviceSP dev, const QRect &rect, const QColor &target)
{
    if (rect.isEmpty()) return;

    const int sr = target.red();
    const int sg = target.green();
    const int sb = target.blue();

    const KoColorSpace *cs = dev->colorSpace();
    KisSequentialIterator it(dev, rect);

    QColor c;
    while (it.nextPixel()) {
        quint8 *pixel = it.rawData();
        cs->toQColor(pixel, &c);

        const int a = c.alpha();
        if (a == 0) continue;                    // nothing to heal on empty pixels

        const int r = c.red();
        const int g = c.green();
        const int b = c.blue();

        const int floor = qMax(r, b);            // green-screen despill floor
        const int spill = g - floor;             // how much green excess is present
        if (spill <= 0) continue;                // not a green pixel — leave it

        // Hard green subtraction: clamp G down to the despill floor (spill -> 0).
        const int gd = floor;

        // Recolour weight: strongly-green pixels go all the way to the sampled
        // colour, faint ones only part way. Repeated incremental passes push the
        // residual green to zero.
        qreal t = spill / GREENHEAL_FULL_RECOLOUR_SPILL;
        if (t > 1.0) t = 1.0;

        const int nr = qBound(0, int(std::lround(r  + (sr - r)  * t)), 255);
        const int ng = qBound(0, int(std::lround(gd + (sg - gd) * t)), 255);
        const int nb = qBound(0, int(std::lround(b  + (sb - b)  * t)), 255);

        c.setRgb(nr, ng, nb, a);                 // alpha preserved exactly
        cs->fromQColor(c, pixel);
    }
}

KisSpacingInformation KisGreenHealOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) {
        return KisSpacingInformation(1.0);
    }

    if (!source()) {
        return KisSpacingInformation(1.0);
    }

    KisBrushSP brush = m_brush;
    if (!brush) return KisSpacingInformation(1.0);

    if (!brush->canPaintFor(info))
        return KisSpacingInformation(1.0);

    qreal scale = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());
    if (checkSizeTooSmall(scale)) return KisSpacingInformation();
    qreal rotation = m_rotationOption.apply(info);
    KisDabShape shape(scale, 1.0, rotation);

    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    QRect dstRect;
    KisFixedPaintDeviceSP dab = m_dabCache->fetchDab(cs, color, info.pos(),
                                                     shape,
                                                     info, 1.0,
                                                     &dstRect);

    if (dstRect.isEmpty()) return KisSpacingInformation(1.0);

    QRect dabRect = dab->bounds();

    // sanity check
    Q_ASSERT(dstRect.size() == dabRect.size());

    // Pull the current layer pixels under the dab into the scratch device.
    KisPainter p(m_tmpDevice);
    p.setCompositeOpId(COMPOSITE_COPY);
    p.bitBltOldData(dstRect.topLeft(), source(), dstRect);

    // Despill + recolour those pixels toward the foreground (Alt-picked) colour.
    applyGreenHeal(m_tmpDevice, dstRect, painter()->paintColor().toQColor());

    // Blit back through the dab, so the soft brush edge / opacity / flow control
    // how strongly the healed result replaces the original (COMPOSITE_COPY keeps
    // alpha exact; overlapping/repeated passes accumulate).
    painter()->bitBltWithFixedSelection(dstRect.x(), dstRect.y(),
                                        m_tmpDevice, dab,
                                        0, 0,
                                        dstRect.x(), dstRect.y(),
                                        dabRect.width(), dabRect.height());

    painter()->renderMirrorMaskSafe(dstRect, m_tmpDevice, 0, 0, dab,
                                    !m_dabCache->needSeparateOriginal());

    return effectiveSpacing(scale, rotation, info);
}

KisSpacingInformation KisGreenHealOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    const qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, info);
}
