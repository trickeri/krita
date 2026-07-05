/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GREENHEALOP_H_
#define KIS_GREENHEALOP_H_

#include "kis_brush_based_paintop.h"
#include <KisStandardOptions.h>
#include <KisRotationOption.h>

class KisPaintInformation;
class KisPainter;

/**
 * Green Heal — a defringe/heal brush for chroma-key leftovers.
 *
 * For every pixel under the dab it does a HARD green subtraction (drives the
 * green spill, G - max(R,B), toward zero) and lerps the pixel toward the
 * current foreground colour (the clean colour picked with Alt), weighted by how
 * green the pixel was. Alpha is left untouched, so the anti-aliased silhouette
 * is preserved and a green edge pixel simply becomes the sampled colour at its
 * existing partial alpha. Painting is incremental, so repeated passes converge
 * the residual green to zero.
 */
class KisGreenHealOp : public KisBrushBasedPaintOp
{

public:

    KisGreenHealOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);
    ~KisGreenHealOp() override;

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:

    /// Despill + recolour every pixel of @p dev inside @p rect toward @p target.
    void applyGreenHeal(KisPaintDeviceSP dev, const QRect &rect, const QColor &target);

    KisPaintDeviceSP m_tmpDevice;
    KisSizeOption m_sizeOption;
    KisRotationOption m_rotationOption;
};

#endif // KIS_GREENHEALOP_H_
