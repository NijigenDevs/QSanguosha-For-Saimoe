/********************************************************************
    Copyright (c) 2013-2015 - Mogara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    Mogara
    *********************************************************************/

#include "banpair.h"
#include "settings.h"

static QSet<BanPair> BanPairSet;
static QSet<QString> AllBanSet;
static QSet<QString> SecondBanSet;

BanPair::BanPair()
{
}

BanPair::BanPair(const QString &first, const QString &second)
    : QPair<QString, QString>(first, second)
{
    if (first > second) {
        qSwap(this->first, this->second);
    }
}

Q_DECLARE_METATYPE(BanPair)

bool BanPair::isBanned(const QString &general)
{
    return AllBanSet.contains(general);
}

bool BanPair::isBanned(const QString &first, const QString &second)
{
    if (SecondBanSet.contains(second))
        return true;

    if (AllBanSet.contains(first) || AllBanSet.contains(second))
        return true;

    BanPair pair(first, second);
    return BanPairSet.contains(pair);
}

void BanPair::loadBanPairs()
{
    AllBanSet.clear();
    SecondBanSet.clear();
    BanPairSet.clear();

    // special cases
    QStringList banlist = Config.value("Banlist/Pairs", "").toStringList();

    foreach (const QString &line, banlist) {
        QStringList names = line.split("+");
        if (names.isEmpty())
            continue;

        QString first = names.at(0).trimmed();
        if (names.length() == 2) {
            QString second = names.at(1).trimmed();
            if (first.isEmpty())
                SecondBanSet.insert(second);
            else {
                BanPair pair(first, second);
                BanPairSet.insert(pair);
            }
        } else if (names.length() == 1) {
            AllBanSet.insert(first);
        }
    }
}

void BanPair::saveBanPairs()
{
    QStringList stream;
    foreach (const QString &banned, AllBanSet)
        stream << banned;
    foreach (const QString &banned, SecondBanSet)
        stream << QString("+%1").arg(banned);
    foreach (const BanPair &pair, BanPairSet)
        stream << QString("%1+%2").arg(pair.first, pair.second);
    Config.setValue("Banlist/Pairs", stream);
}

const QSet<QString> BanPair::getAllBanSet()
{
    return AllBanSet;
}

const QSet<QString> BanPair::getSecondBanSet()
{
    return SecondBanSet;
}

const QSet<BanPair> BanPair::getBanPairSet()
{
    return BanPairSet;
}

