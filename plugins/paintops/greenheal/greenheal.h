/*
 *  SPDX-FileCopyrightText: 2026 Troy Maynard <troy@nuldrums.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _GREENHEAL_H_
#define _GREENHEAL_H_

#include <QObject>
#include <QVariant>

class GreenHeal : public QObject
{
    Q_OBJECT
public:
    GreenHeal(QObject *parent, const QVariantList &);
    ~GreenHeal() override;
};

#endif
